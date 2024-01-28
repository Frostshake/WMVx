#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_WMVx.h"
#include "core/modeling/Scene.h"
#include "core/game/GameClientInfo.h"
#include "core/database/WOTLKGameDatabase.h"
#include "WMVxSettings.h"
#include "core/utility/Logger.h"
#include "Debounce.h"

class WMVx : public QMainWindow
{
    Q_OBJECT

public:
    WMVx(QWidget *parent = nullptr);
    ~WMVx();

signals:
    void gameConfigLoaded(core::GameDatabase* gameDB, core::GameFileSystem* gameFS, core::ModelSupport& modelSupport);

public slots:
    void updateMemoryUsage();
    void updateStatus(QString status);
    void onGameClientChosen(core::GameClientInfo clientInfo);

protected:

    virtual void resizeEvent(QResizeEvent* event);

private:
    Ui::WMVxClass ui;

    void openClientChoiceDialog();
    void unloadGameClient();

    inline void setupControls();

    void sceneLoad();
    void sceneSave();


    QLabel* labelStatus;
    QLabel* labelClientInfo;
    QLabel* labelCanvasSize;
    QLabel* labelMemory;
    uint64_t getMemoryUsage();

    QProgressDialog* clientProgressDialog;

    Debounce* delayedWindowResize;

    core::Scene* scene;

    std::optional<core::GameClientInfo> gameClientInfo;
    core::ModelSupport modelSupport;
    std::unique_ptr<core::GameFileSystem> gameFS;
    std::unique_ptr<core::GameDatabase> gameDB;

    std::atomic<bool> isLoadingClient;
};
