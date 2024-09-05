#include "stdafx.h"
#include "WMVx.h"
#include <QColorDialog>
#include <psapi.h>
#include "AboutDialog.h"
#include "SettingsDialog.h"
#include "ClientChoiceDialog.h"
#include "DevTools.h"
#include "TextureTool.h"
#include "WMVxVideoCapabilities.h"
#include "ExportImageDialog.h"
#include "Export3dDialog.h"
#include "core/modeling/SceneIO.h"
#include "Build.h"
#include <QProgressDialog>
#include <QtConcurrent>

using namespace core;

WMVx::WMVx(QWidget* parent)
    : QMainWindow(parent),
    modelSupport(core::NullModelSupport), isInitialised(false)
{
    VideoCapabilities::boot(this);
    Log::boot(this);
    Settings::boot(this);
    Settings::instance()->load();

    clientProgressDialog = new QProgressDialog(this);
    clientProgressDialog->close();
    connect(clientProgressDialog, &QProgressDialog::canceled, [&]() {
        // if the clientProgressDialog dialog is closed while still loading (e.g via user), then quit.
        if (isLoadingClient) {
            close();
        }
    });

    scene = new Scene(this);
    gameFS = nullptr;
    gameDB = nullptr;

    isLoadingClient = false;

    assetCache = new AssetSupportCache(this);

    ui.setupUi(this);

#ifndef _DEBUG
    ui.dockLog->hide();
#endif

    {
        auto old_window_size = Settings::get<QSize>(config::app::window_size);
        if (Settings::get<bool>(config::app::window_maximized)) {
            setWindowState(Qt::WindowState::WindowMaximized);
        } else if (old_window_size.isValid()) {
            if (screen()->size().width() >= old_window_size.width() && 
                screen()->size().height() >= old_window_size.height()) {
                resize(old_window_size);
            }
        }
    }

    connect(Log::instance(), &Logger::entryAdded, [&](QString message) {
        auto cursor = ui.plainTextLog->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui.plainTextLog->setTextCursor(cursor);

        ui.plainTextLog->insertPlainText(QString('\n') + message);
        ui.plainTextLog->verticalScrollBar()->setSliderPosition(ui.plainTextLog->verticalScrollBar()->maximum());
    });

    ui.plainTextLog->insertPlainText("Live log loaded.");

    Log::message("System Info:");
    Log::message("Arch: " + QSysInfo::currentCpuArchitecture());
    Log::message("OS: " + QSysInfo::prettyProductName());
    Log::message("WMVx: " WMVX_BUILD);
    Log::message("QT: " + QString("%1.%2").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR));
    Log::message("Initialising...");
       
    delayedWindowResize = new Debounce(this);
    connect(delayedWindowResize, &Debounce::triggered, [&]() {
        const auto window_size = this->size();
        if (Settings::instance() != nullptr) {
            Settings::instance()->set(config::app::window_maximized, windowState() == Qt::WindowState::WindowMaximized);
            Settings::instance()->set(config::app::window_size, window_size);
            Settings::instance()->save();
        }
    });

    setupControls();

    ui.libraryFilesControl->onSceneLoaded(scene);
    ui.libraryNpcsControl->onSceneLoaded(scene);
    ui.renderWidget->onSceneLoaded(scene);
    ui.sceneControl->onSceneLoaded(scene);
    ui.modelControl->onSceneLoaded(scene);
    ui.renderControl->onSceneLoaded(scene);
    ui.characterControl->onSceneLoaded(scene);
    ui.animationControl->onSceneLoaded(scene);

    connect(scene, &Scene::modelSelectionChanged, this, &WMVx::onSelectionChanged);

    connect(this, &WMVx::gameConfigLoaded, [&](auto&& ...args) {
        ui.modelControl->onGameConfigLoaded(args...);
        ui.renderControl->onGameConfigLoaded(args...);
        ui.libraryFilesControl->onGameConfigLoaded(args...);
        ui.libraryNpcsControl->onGameConfigLoaded(args...);
        ui.animationControl->onGameConfigLoaded(args...);
        ui.characterControl->onGameConfigLoaded(args...); 
    });

    connect(ui.renderWidget, &RenderWidget::resized, [&]() {
        labelCanvasSize->setText(QString("Canvas: %1 x %2 ").arg(ui.renderWidget->width()).arg(ui.renderWidget->height()));
    });

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &WMVx::updateMemoryUsage);
    timer->setInterval(1000 * 5);
    timer->start();
    
    updateMemoryUsage();

    // slight delay opening the popup dialog looks to solve positioning issues when the window first opens.
    QTimer::singleShot(500, this, &WMVx::openClientChoiceDialog);
    
    Log::message("WMVx Loaded.");
    isInitialised = true;
}

WMVx::~WMVx()
{
    Log::message("WMVx destroyed.");
}

void WMVx::updateMemoryUsage() {
    auto memoryMb = getMemoryUsage() >> 20;
    auto memoryStr = QString("Memory: %1MB ").arg(memoryMb);
    labelMemory->setText(QString("Memory: %1MB ").arg(memoryMb));
}

uint64_t WMVx::getMemoryUsage() {
#ifdef _WINDOWS
    auto process_id = GetCurrentProcessId();
    HANDLE hProcess;
    PROCESS_MEMORY_COUNTERS pmc;

    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
        PROCESS_VM_READ,
        FALSE, process_id);

    if (hProcess != NULL) {
        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            return pmc.WorkingSetSize;
        }
    }
#else
#error unsupported platform
#endif

    return 0;
}

void WMVx::updateStatus(QString status) {
    labelStatus->setText("Status: " + status + " ");
}
void WMVx::onGameClientChosen(core::GameClientInfo clientInfo) {

    assert(isLoadingClient == false);
    if (isLoadingClient) {
        Log::message("Already loading client. Cannot attempt loading new client.");
        return;
    }

    gameClientInfo.emplace(clientInfo);

    labelClientInfo->setText("Client: " + QString::fromStdString(gameClientInfo->environment.version) + " " + gameClientInfo->environment.locale + " ");

    Log::message(
        QString("Client profile:\n%1 %2")
        .arg(QString::fromStdString(gameClientInfo->profile.shortName))
        .arg(QString::fromStdString(gameClientInfo->profile.versionString))
    );
    Log::message(QString("Client Environment: \n%1\n%2")
        .arg(QString::fromStdString(gameClientInfo->environment.version))
        .arg(gameClientInfo->environment.directory)
    );

    //TODO only update fields of the client profile requires it.
    const auto update_status = assetCache->status(clientInfo.environment.version);
    if (update_status != AssetSupportCache::UP_TO_DATE) {
        promptSupportUpdate(update_status);
    }
 
    updateStatus("Reading client files");

    std::shared_ptr<GameClientAdaptor> gameAdaptor = makeGameClientAdaptor(*gameClientInfo);        
    if (gameAdaptor == nullptr) {
        
        updateStatus("Client version unsupported");
        unloadGameClient();
        QMessageBox::warning(this, "Client Version Error", "Detected client version is not supported.", QMessageBox::Ok);

        return;
    }

    clientProgressDialog->setModal(true);
    clientProgressDialog->setWindowTitle("Loading Client");
    clientProgressDialog->setRange(0, 6);
    clientProgressDialog->setLabelText("Loading...");
    clientProgressDialog->setValue(0);
    clientProgressDialog->setCancelButton(nullptr);
    clientProgressDialog->show();

    isLoadingClient = true;

    QtConcurrent::run([&, gameAdaptor]() {

        try {
            QMetaObject::invokeMethod(this, [&] {
                clientProgressDialog->setLabelText("Loading filesystem...");
            });

            gameFS = gameAdaptor->filesystem(gameClientInfo->environment);
            auto fs_future = gameFS->load();

            QMetaObject::invokeMethod(this, [&] {
                clientProgressDialog->setValue(1);
                clientProgressDialog->setLabelText("Loading database...");
            });

            gameDB = gameAdaptor->database();

            QMetaObject::invokeMethod(this, [&] {
                clientProgressDialog->setValue(2);
                clientProgressDialog->setLabelText("Opening database...");
            });

            gameDB->load(gameFS.get());

            QMetaObject::invokeMethod(this, [&] {
                clientProgressDialog->setValue(3);
                clientProgressDialog->setLabelText("Initialising factory...");
            });

            modelSupport = gameAdaptor->modelSupport();

            QMetaObject::invokeMethod(this, [&] {
                clientProgressDialog->setValue(4);
                clientProgressDialog->setLabelText("Finalising Filesystem...");
            });
            
            if (fs_future.valid()) {
                fs_future.get();
            }

            QMetaObject::invokeMethod(this, [&] {
                clientProgressDialog->setValue(5);
                clientProgressDialog->setLabelText("Finishing...");
            });
        }
        catch (WDBR::WDBReaderException& e) {
            Log::message("Exception caught loading game config:");
            Log::message(e.what());
            Log::message(QString("Code: %1").arg(e.getErrorCode().value()));
        }
        catch (std::exception& e) {
            QMetaObject::invokeMethod(this, [&, e] {
                Log::message("Exception caught loading game config:");
                Log::message(e.what());

                unloadGameClient();

                updateStatus("Client failure");
                QMessageBox::warning(
                    this, 
                    "Client Data Error", 
                    QString("An error occured while loading client data.\n%1").arg(e.what()),
                    QMessageBox::Ok
                );

                isLoadingClient = false;
                clientProgressDialog->close();
            });

            return;
        } 

        // if it got this far then loading has been successful
        QMetaObject::invokeMethod(this, [&] {
            ui.actionLoad_Client->setDisabled(true);
            ui.actionUpdate_Support_Files->setDisabled(true);

            Settings::instance()->set(config::client::game_folder, gameClientInfo->environment.directory);
            Settings::instance()->save();

            Log::message("Game config loaded.");
            updateStatus("Loaded");

            isLoadingClient = false;
            clientProgressDialog->setValue(6);
            clientProgressDialog->close();

            emit gameConfigLoaded(gameDB.get(), gameFS.get(), modelSupport);
            
        });
    });
  
}

void WMVx::onSelectionChanged(const core::Scene::Selection& selection)
{
    if (selection.component) {
        labelSelection->setText("Selection: " + selection.component->getMetaLabel() + " ");
        labelSelection->setToolTip(selection.component->getMetaGameFileInfo().toString());
    }
    else {
        labelSelection->setText("Selection: N/A ");
        labelSelection->setToolTip("");
    }
}

void WMVx::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (!isInitialised) {
        return;
    }

    if (event->oldSize().width() > 0 && event->oldSize().height() > 0) {
        delayedWindowResize->absorb();
    }
}

void WMVx::promptSupportUpdate(uint32_t status)
{
    QString last_update_str = "Unknown";
    auto last_update = assetCache->lastUpdated();
    if (last_update) {
        last_update_str = QLocale().toString(last_update.value(), QLocale::ShortFormat);
    }

    QString message = QString("Update support files? \nUsing the latest support files improves game client compatability. \nLast updated: %1").arg(last_update_str);
    if (QMessageBox::information(this, "Update", message, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
        updateStatus("Updating support files");

        QProgressDialog* progress = new QProgressDialog(this);
        progress->setAttribute(Qt::WA_DeleteOnClose);
        progress->setWindowTitle("Updating");
        progress->setLabelText("Updating support files...");
        progress->setCancelButton(nullptr);
        progress->setModal(true);

        std::stop_source stop_source;

        disconnect(assetCache, &AssetSupportCache::progress, 0, 0);

        connect(assetCache, &AssetSupportCache::progress, [&progress, &stop_source](const AssetSupportCache::Update* update, uint64_t received, uint64_t total) {
            if (!stop_source.stop_requested()) {
                progress->setLabelText("Updating: " + update->name);
                progress->setMaximum(total);
                progress->setValue(received);
            }
        });

        connect(progress, &QProgressDialog::canceled, [&]() {
            stop_source.request_stop();
        });

        progress->setMinimum(0);
        progress->setMaximum(0);
        progress->setValue(0);
        progress->setAutoClose(false);
        progress->setAutoReset(false);
        progress->show();

        bool all_updated = assetCache->fetchUpdates(status, stop_source.get_token());        

        if (!stop_source.stop_requested()) {
            progress->close();
        }

        if (!all_updated) {
            QMessageBox::warning(this, "Update Error", "Not all files could be updated - check log for details.");
        }
    }
}

void WMVx::openClientChoiceDialog() {
    auto clientDialog = new ClientChoiceDialog(this);
    clientDialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(clientDialog, &ClientChoiceDialog::chosen, this, &WMVx::onGameClientChosen, Qt::QueuedConnection);
    connect(clientDialog, &ClientChoiceDialog::rejected, [&]() {
        gameClientInfo = std::nullopt;
        labelClientInfo->setText("Client: N/A ");
        updateStatus("Client files missing");
        unloadGameClient();
    });
    clientDialog->show();
}

void WMVx::unloadGameClient()
{
    modelSupport = NullModelSupport;

    emit gameConfigLoaded(nullptr, nullptr, modelSupport);

    gameFS.reset();
    gameDB.reset();
}

void WMVx::setupControls() {
    labelStatus = new QLabel("Status: Initialising... ", this);
    labelClientInfo = new QLabel("Client: - ", this);
    labelCanvasSize = new QLabel(QString("Canvas: %1 x %2 ").arg(ui.renderWidget->width()).arg(ui.renderWidget->height()), this); 
    labelMemory = new QLabel("Memory: ? ", this);
    labelSelection = new QLabel("Selected: N/A ", this);

    ui.statusBar->addWidget(labelStatus);
    ui.statusBar->addWidget(labelClientInfo);
    ui.statusBar->addWidget(labelCanvasSize);
    ui.statusBar->addWidget(labelMemory);
    ui.statusBar->addWidget(labelSelection);

    // file tab

    connect(ui.actionSettings, &QAction::triggered, [&]() {
        auto settingsDialog = new SettingsDialog(this);
        settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
        settingsDialog->show();
     });

    connect(ui.actionLoad_Client, &QAction::triggered, this, &WMVx::openClientChoiceDialog);
    connect(ui.actionUpdate_Support_Files, &QAction::triggered, [this]() {
        promptSupportUpdate(AssetSupportCache::ALL_OUTDATED);
    });

    connect(ui.actionView_Log, &QAction::triggered, [&]() {
        auto path = Log::instance()->getFilePath();
        if (QFile::exists(path)) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(path));
        }
        else {
            QMessageBox::warning(this, "Unable to open file", "Cannot open " + path);
        }
    });

    connect(ui.actionExport_Image, &QAction::triggered, [&]() {
        auto exporter = new ExportImageDialog(ui.renderWidget, this);
        exporter->setAttribute(Qt::WA_DeleteOnClose);
        exporter->show();
    });

    connect(ui.actionExport_3D, &QAction::triggered, [&]() {
        auto exporter = new Export3dDialog(gameDB.get(), scene, this);
        exporter->setAttribute(Qt::WA_DeleteOnClose);
        exporter->show();
    });

    connect(ui.actionSceneLoad, &QAction::triggered, this, &WMVx::sceneLoad);
    connect(ui.actionSceneSave, &QAction::triggered, this, &WMVx::sceneSave);

    connect(ui.actionReset_Layout, &QAction::triggered, [&]() {
        ui.dockLibrary->show();
        ui.dockAnimation->show();
        ui.dockScene->show();
        ui.dockCharacter->show();
        ui.dockModel->show();
        ui.dockRender->show();
        ui.dockLog->hide();
    });

    connect(ui.actionExit, &QAction::triggered, this, &QWidget::close);

    // view tab

    connect(ui.actionShow_Library_Panel, &QAction::triggered, ui.dockLibrary, &QDockWidget::show);
    connect(ui.actionShow_Animation_Panel, &QAction::triggered, ui.dockAnimation, &QDockWidget::show);
    connect(ui.actionShow_Scene_Panel, &QAction::triggered, ui.dockScene, &QDockWidget::show);
    connect(ui.actionShow_Characters_Panel, &QAction::triggered, ui.dockCharacter, &QDockWidget::show);
    connect(ui.actionShow_Model_Panel, &QAction::triggered, ui.dockModel, &QDockWidget::show);
    connect(ui.actionShow_Render_Panel, &QAction::triggered, ui.dockRender, &QDockWidget::show);
    connect(ui.actionShow_Log_Panel, &QAction::triggered, ui.dockLog, &QDockWidget::show);

    connect(ui.actionToggle_Grid, &QAction::triggered, [&]() {
        scene->showGrid = !scene->showGrid;
    });

    connect(ui.actionBGColor, &QAction::triggered, [&]() {
        QColor color = QColorDialog::getColor(
            Settings::get<QColor>(config::app::background_color), 
            this, 
            "Choose Background Color",
            QColorDialog::ColorDialogOption::ShowAlphaChannel
        );
 
        if (color.isValid()) {
            Settings::instance()->set(config::app::background_color, color);
            Settings::instance()->save();

            auto newColor = ColorRGBA<float>(color.redF(), color.greenF(), color.blueF());
            ui.renderWidget->setBackground(newColor);
        }
    });

    connect(ui.actionBGTransparent, &QAction::triggered, [&]() {
        ui.renderWidget->setBackground(ColorRGBA<float>(0, 0, 0, 0));
    });

    connect(ui.actionCamera_Reset, &QAction::triggered, ui.renderWidget, &RenderWidget::resetCamera);

    connect(ui.actionOpen_Dev_Tools, &QAction::triggered, [&]() {
        auto tools = new DevTools(this);
        tools->setAttribute(Qt::WA_DeleteOnClose);
        tools->onSceneLoaded(scene);
        tools->show();
    });

    connect(ui.actionOpen_Texture_Tool, &QAction::triggered, [&]() {
        if (!gameFS) {
            return;
        }

        auto tool = new TextureTool(this, gameFS.get());
        tool->setAttribute(Qt::WA_DeleteOnClose);
        tool->show();
    });

    // about tab

    connect(ui.actionHelp, &QAction::triggered, [&]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Frostshake/WMVx"));
    });

    connect(ui.actionAbout_WMVx, &QAction::triggered, [&]() {
        auto aboutDialog = new AboutDialog(this);
        aboutDialog->setAttribute(Qt::WA_DeleteOnClose);
        aboutDialog->show();
    });
}

void WMVx::sceneLoad()
{
    if (!gameClientInfo.has_value()) {
        return;
    }

    auto inFile = QFileDialog::getOpenFileName(
        this, 
        "Open Scene", 
        Settings::get(config::exporter::last_scene_directory), 
        "SCENE (*.wmvx)"
    );

    if (!inFile.isNull()) {
        try {
            SceneIO(gameClientInfo.value())
                .setDatabase(gameDB.get())
                .setFilesystem(gameFS.get())
                .setModelFactory(modelSupport.m2Factory)
                .setScene(scene)
                .setAttachmentProviderFactory(modelSupport.attachmentCustomizationProviderFactory)
                .load(inFile);
        }
        catch (std::exception e) {
            Log::message("Exception caught loading scene:");
            Log::message(e.what());
            QMessageBox::warning(this,
                "Scene error",
                QString("An error occured while loading scene data. \n%1").arg(e.what()),
                QMessageBox::Ok
            );
        }

        QFileInfo file_info(inFile);
        Settings::instance()->set(config::exporter::last_scene_directory, file_info.dir().absolutePath());
        Settings::instance()->save();
    }
}

void WMVx::sceneSave()
{
    if (!gameClientInfo.has_value()) {
        return;
    }

    auto outFile = QFileDialog::getSaveFileName(
        this,
        "Save Scene", 
        Settings::get(config::exporter::last_scene_directory),
        "SCENE (*.wmvx)");

    if (!outFile.isNull()) {
        try {
            SceneIO(gameClientInfo.value())
                .setDatabase(gameDB.get())
                .setFilesystem(gameFS.get())
                .setScene(scene)
                .save(outFile);
        }
        catch (std::exception e) {
            Log::message("Exception caught saving scene:");
            Log::message(e.what());
            QMessageBox::warning(this, 
                "Scene error", 
                QString("An error occured while saving scene data. \n%1").arg(e.what()),
                QMessageBox::Ok
            );
        }

        QFileInfo file_info(outFile);
        Settings::instance()->set(config::exporter::last_scene_directory, file_info.dir().absolutePath());
        Settings::instance()->save();
    }
}
