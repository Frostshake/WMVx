#include "stdafx.h"
#include "LibraryNpcsControl.h"
#include <QtConcurrent>

using namespace core;

LibraryNpcsControl::LibraryNpcsControl(QWidget *parent)
	: WidgetUsesScene(),
	WidgetUsesGameClient(),
	QWidget(parent)
{
	ui.setupUi(this);

	ui.listWidgetNpcs->setDisabled(true);
	ui.lineEditSearch->setDisabled(true);

	delayedSearch = new Debounce(this);
	connect(delayedSearch, &Debounce::triggered, [&]() {
		auto search = ui.lineEditSearch->text();
		if (search.length() == 0) {
			for (auto i = 0; i < ui.listWidgetNpcs->count(); i++) {
				ui.listWidgetNpcs->item(i)->setHidden(false);
			}
		}
		else {
			bool is_id_search;
			int id_search = search.toInt(&is_id_search);

			if (is_id_search) {
				//searching by item id
				for (auto i = 0; i < ui.listWidgetNpcs->count(); i++) {
					uint32_t item_id = ui.listWidgetNpcs->item(i)->data(1).toInt();;
					ui.listWidgetNpcs->item(i)->setHidden(item_id != id_search);
				}
			}
			else {
				//search item name
				for (auto i = 0; i < ui.listWidgetNpcs->count(); i++) {
					auto text = ui.listWidgetNpcs->item(i)->text();
					ui.listWidgetNpcs->item(i)->setHidden(!text.contains(search, Qt::CaseInsensitive));
				}
			}
		}
	});

	connect(ui.lineEditSearch, &QLineEdit::textChanged, delayedSearch, &Debounce::absorb);
	connect(ui.listWidgetNpcs, &QListWidget::itemSelectionChanged, [&]() {
		if(ui.listWidgetNpcs->selectedItems().length() == 1) {
			auto selected = ui.listWidgetNpcs->selectedItems()[0];
			auto record_id = selected->data(Qt::UserRole).toInt();

			if (gameDB != nullptr && gameFS != nullptr) {


				for (const auto& npcAdaptor : gameDB->npcsDB->all()) {
					if (npcAdaptor->getId() == record_id) {

						auto display_info = gameDB->creatureDisplayDB->findById(npcAdaptor->getModelId());
						if (display_info != nullptr) {							

							auto model = gameDB->creatureModelDataDB->findById(display_info->getModelId());
							if (model != nullptr) {
								try {
									auto model_file_uri = model->getModelUri();
									if (model_file_uri.isPath()) {;
										model_file_uri = GameFileUri::replaceExtension(model_file_uri.getPath(), "mdx", "m2");
									}

									Log::message("Loading npc: " + QString::number(record_id) + " / " + model_file_uri.toString());

									auto m = std::make_unique<Model>(Model(modelSupport.modelFactory));
									m->initialise(model_file_uri, gameFS, gameDB, scene->textureManager);

									//TODO test with 'character' type npcs
									//TODO handle npc attachments, eg weapons
									//TODO lookup with CreatureDisplayInfoExtra

									scene->addModel(std::move(m));
								}
								catch (std::exception e) {
									Log::message("Exception caught loading npc:");
									Log::message(e.what());
									QMessageBox::warning(this, "Model Data Error", "An error occured while loading npc data.", QMessageBox::Ok);
								}

							}
						}
							
						break;
					}
				}


			}
		}
	});
}

LibraryNpcsControl::~LibraryNpcsControl()
{}

bool LibraryNpcsControl::event(QEvent* event) {
		
	if (event->type() == QEvent::Show) {
		if (gameDB != nullptr && gameFS != nullptr) {
			loadNpcs();
		}
	}

	return QWidget::event(event);
}

void LibraryNpcsControl::onGameConfigLoaded(GameDatabase* db, GameFileSystem* fs, ModelSupport& ms)
{
	WidgetUsesGameClient::onGameConfigLoaded(db, fs, ms);

	if (gameDB != nullptr && gameFS != nullptr) {
		if (this->isVisible()) {
			loadNpcs();
		}
	}
}

void LibraryNpcsControl::loadNpcs()
{
	if (ui.listWidgetNpcs->count() > 0) {
		//already loadded, ignore;
		return;
	}

	ui.listWidgetNpcs->setUniformItemSizes(true);
	ui.listWidgetNpcs->setDisabled(true);
	ui.listWidgetNpcs->setUpdatesEnabled(false);

	QtConcurrent::run([&]() {
		for (const auto& adaptor : gameDB->npcsDB->all()) {
			auto item = new QListWidgetItem(ui.listWidgetNpcs);
			item->setText(adaptor->getName());
			item->setData(Qt::UserRole, QVariant(adaptor->getId()));
			ui.listWidgetNpcs->addItem(item);
		}

		ui.listWidgetNpcs->sortItems();
		QMetaObject::invokeMethod(this, [&] {
			ui.listWidgetNpcs->setUpdatesEnabled(true);
			ui.listWidgetNpcs->setDisabled(false);
			ui.lineEditSearch->setDisabled(false);
		});
	});
}
