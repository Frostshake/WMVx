#include "stdafx.h"
#include "LibraryNpcsControl.h"
#include <QtConcurrent>
#include <ranges>
#include <functional>
#include <algorithm>
#include <array>

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

			if (search.startsWith("https://")) {
				//handle pasting in website urls

				const std::array<QRegularExpression, 3> patterns {
					QRegularExpression{"https:\\/\\/classicdb\\.ch\\/\\?npc=(\\d+)"},
					QRegularExpression{"https:\\/\\/www\\.wowhead\\.com\\/npc=(\\d+)\\/?.*"},
					QRegularExpression{"https:\\/\\/www\\.wowhead\\.com\\/\\w+\\/npc=(\\d+)\\/?.*"}
				};

				for (const auto& regex : patterns) {
					QRegularExpressionMatch match = regex.match(search);
					if (match.hasMatch()) {
						search = match.captured(1);
						break;
					}
				}

			}

			bool is_id_search;
			uint id_search = search.toUInt(&is_id_search);

			if (is_id_search) {
				//searching by item id
				for (auto i = 0; i < ui.listWidgetNpcs->count(); i++) {
					uint32_t item_id = ui.listWidgetNpcs->item(i)->data(Qt::UserRole).toInt();;
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

			Log::message(QString("Selected npc id: %1").arg(record_id));

			if (gameDB != nullptr && gameFS != nullptr) {

				const auto* npcAdaptor = gameDB->npcsDB->findById(record_id);
				if (npcAdaptor != nullptr) {

					const auto* display_info = gameDB->creatureDisplayDB->findById(npcAdaptor->getModelId());
					if (display_info != nullptr) {

						const auto* model = gameDB->creatureModelDataDB->findById(display_info->getModelId());
						if (model != nullptr) {
							try {
								auto model_file_uri = model->getModelUri();
								if (model_file_uri.isPath()) {
									model_file_uri = GameFileUri::replaceExtension(model_file_uri.getPath(), "mdx", "m2");
								}

								Log::message("Loading npc: " + QString::number(record_id) + " / " + model_file_uri.toString());

								auto m = std::make_unique<Model>(modelSupport.modelFactory);
								m->initialise(model_file_uri, gameFS, gameDB, scene->textureManager);

								const auto name = npcAdaptor->getName();
								if (name.length() > 0) {
									m->setMetaName(QString("%1 [%2]").arg(name).arg(record_id));
								}

								const auto* extra = display_info->getExtra();
								const auto& info = m->getCharacterDetails();
								if (extra != nullptr && info.has_value()) {
									std::unique_ptr<core::CharacterCustomizationProvider> charCustomProvider = modelSupport.characterCustomizationProviderFactory(gameFS, gameDB);
									charCustomProvider->initialise(*info);

									CharacterCustomizations custom_options;
									const auto available_options = charCustomProvider->getAvailableOptions();
									for (const auto& opt : available_options) {
										custom_options[opt.first] = 0;
									}

									if (custom_options.contains(LegacyCharacterCustomization::Name::Skin)) {
										custom_options[LegacyCharacterCustomization::Name::Skin] = extra->getSkinId();
									}

									if (custom_options.contains(LegacyCharacterCustomization::Name::Face)) {
										custom_options[LegacyCharacterCustomization::Name::Face] = extra->getFaceId();
									}

									if (custom_options.contains(LegacyCharacterCustomization::Name::HairStyle)) {
										custom_options[LegacyCharacterCustomization::Name::HairStyle] = extra->getHairStyleId();
									}

									if (custom_options.contains(LegacyCharacterCustomization::Name::HairColor)) {
										custom_options[LegacyCharacterCustomization::Name::HairColor] = extra->getHairColorId();
									}

									if (custom_options.contains(LegacyCharacterCustomization::Name::FacialStyle)) {
										custom_options[LegacyCharacterCustomization::Name::FacialStyle] = extra->getFacialHairId();
									}

									if (!charCustomProvider->apply(m.get(), *info, custom_options)) {
										Log::message("NPC customization invalid.");
									}

									const auto item_display_ids = extra->getItemDisplayIds();
									for (const auto& item_display : item_display_ids) {
										const auto* display = gameDB->itemDisplayDB->findById(item_display.second);
										if (display != nullptr) {
											for (const auto char_slot : Mapping::CharacterSlotItemInventory) {
												if (std::ranges::count(char_slot.second, item_display.first) > 0) {
													m->characterEquipment.insert_or_assign(char_slot.first, CharacterItemWrapper::make(item_display.first, display));
													break;
												}
											}
										}
										else {
											Log::message("Unable to find npc item - " + QString::number(item_display.second));
										}

									}
								}

								scene->addModel(std::move(m));
							}
							catch (std::exception e) {
								Log::message("Exception caught loading npc:");
								Log::message(e.what());
								QMessageBox::warning(this,
									"Model Data Error",
									QString("An error occured while loading npc data. \n%1").arg(e.what()),
									QMessageBox::Ok);
							}

						}
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
