#include "stdafx.h"
#include "CharacterControl.h"
#include "EquipmentChoiceDialog.h"
#include "CustomTabardDialog.h"
#include "EffectChoiceDialog.h"
#include "core/utility/Logger.h"
#include "core/modeling/WOTLKModel.h"
#include <ranges>
#include <functional>
#include <algorithm>

using namespace core;

CharacterControl::CharacterControl(QWidget* parent)
	: WidgetUsesScene(),
	WidgetUsesGameClient(),
	QWidget(parent)
{
	ui.setupUi(this);

	model = nullptr;

	isLoadingModel = false;
	isRandomising = false;

	controlMap[CharacterSlot::HEAD] = { ui.pushButtonHead, ui.labelHead };
	controlMap[CharacterSlot::SHOULDER] = { ui.pushButtonShoulder, ui.labelShoulder };
	controlMap[CharacterSlot::SHIRT] = { ui.pushButtonShirt, ui.labelShirt };
	controlMap[CharacterSlot::CHEST] = { ui.pushButtonChest, ui.labelChest };
	controlMap[CharacterSlot::BELT] = { ui.pushButtonBelt, ui.labelBelt };
	controlMap[CharacterSlot::PANTS] = { ui.pushButtonLegs, ui.labelLegs };
	controlMap[CharacterSlot::BOOTS] = { ui.pushButtonBoots, ui.labelBoots };
	controlMap[CharacterSlot::BRACERS] = { ui.pushButtonBracers, ui.labelBracers };
	controlMap[CharacterSlot::GLOVES] = { ui.pushButtonGloves, ui.labelGloves };
	controlMap[CharacterSlot::CAPE] = { ui.pushButtonCape, ui.labelCape };
	controlMap[CharacterSlot::HAND_RIGHT] = { ui.pushButtonRightHand, ui.labelRightHand };
	controlMap[CharacterSlot::HAND_LEFT] = { ui.pushButtonLeftHand, ui.labelLeftHand };
	controlMap[CharacterSlot::TABARD] = { ui.pushButtonTabard, ui.labelTabard };

	for (auto& item : controlMap) {
		connect(item.second.button, &QPushButton::pressed, [&]() {
			openChoiceDialog(item.first);
		});
	}

	connect(ui.pushButtonCustomTabard, &QPushButton::pressed, [&]() {
		auto customDialog = new CustomTabardDialog(gameDB, gameFS, tabardCustomizationProvider.get(), this);
		customDialog->setAttribute(Qt::WA_DeleteOnClose);
		connect(customDialog, &CustomTabardDialog::chosen, [&](core::TabardCustomizationOptions tabard) {
	
			model->tabardCustomization = tabardCustomizationProvider->getData(tabard);
			bool customizable_tabard_equiped = false;
			auto possible_tabard_ids = std::views::keys(tabardCustomizationProvider->getTieredCustomTabardItemIds());

			if (model->characterEquipment.contains(core::CharacterSlot::TABARD)) {
				auto current_tabard_item_id = model->characterEquipment[core::CharacterSlot::TABARD]->getId();
				customizable_tabard_equiped = std::any_of(possible_tabard_ids.begin(), possible_tabard_ids.end(), 
					[&current_tabard_item_id](uint32_t possible) {
						return possible = current_tabard_item_id;
					});
			}

			if (!customizable_tabard_equiped) {
				const auto* default_tabard = gameDB->itemsDB->findById(possible_tabard_ids.front());
				if (default_tabard != nullptr) {
					model->characterEquipment[core::CharacterSlot::TABARD] = default_tabard;
				}
			}

			updateModel();
		});
		customDialog->show();
	});

	connect(ui.pushButtonRandomise, &QPushButton::pressed, [&]() {
		if (model != nullptr && !isLoadingModel) {
			isRandomising = true;
			for (const auto& custom : customizationSizes) {
				QComboBox* combo = getCustomizationControl(QString::fromStdString(custom.first));
				if (combo) {
					randomiseComboBox(combo);
				}
			}
			isRandomising = false;

			applyCustomizations();
			updateModel();
		}
	});

	connect(ui.comboBoxEyeGlow, &QComboBox::currentIndexChanged, [&](int index) {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.eyeGlow = static_cast<CharacterRenderOptions::EyeGlow>(index);
			updateModel();
		}
	});

	connect(ui.checkBoxUnderWear, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.showUnderWear = ui.checkBoxUnderWear->isChecked();
			updateModel();
		}
	});

	connect(ui.checkBoxEars, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.showEars = ui.checkBoxEars->isChecked();
			updateModel();
		}
	});

	connect(ui.checkBoxFeet, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.showFeet = ui.checkBoxFeet->isChecked();
			updateModel();
		}
	});

	connect(ui.checkBoxHair, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.showHair = ui.checkBoxHair->isChecked();
			updateModel();
		}
	});

	connect(ui.checkBoxFacialHair, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.showFacialHair = ui.checkBoxFacialHair->isChecked();
			updateModel();
		}
	});

	connect(ui.checkBoxSheatheWeapons, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr && !isLoadingModel) {
			model->characterOptions.sheatheWeapons = ui.checkBoxSheatheWeapons->isChecked();


			for (auto* attachment : model->getAttachments()) {
				if ((attachment->characterSlot == CharacterSlot::HAND_LEFT || 
					attachment->characterSlot == CharacterSlot::HAND_RIGHT) &&
					model->characterEquipment.contains(attachment->characterSlot)) {

					const auto& item = model->characterEquipment.at(attachment->characterSlot);
					auto positions = getAttachmentPositions(attachment->characterSlot, item);
					assert(positions.size() == 1);

					model->setAttachmentPosition(attachment, positions[0]);
				}
			}
		}
	});

	ui.comboBoxEyeGlow->addItem("None", QVariant(CharacterRenderOptions::EyeGlow::NONE));
	ui.comboBoxEyeGlow->addItem("Normal", QVariant(CharacterRenderOptions::EyeGlow::NORMAL));
	ui.comboBoxEyeGlow->addItem("Death Knight", QVariant(CharacterRenderOptions::EyeGlow::DEATH_KNIGHT));
	ui.comboBoxEyeGlow->setCurrentIndex(1);


	connect(ui.pushButtonEffectRight, &QPushButton::pressed, [&]() {
		if (model != nullptr && !isLoadingModel) {
			openEnchantDialog(CharacterSlot::HAND_RIGHT);
		}
	});

	connect(ui.pushButtonEffectLeft, &QPushButton::pressed, [&]() {
		if (model != nullptr && !isLoadingModel) {
			openEnchantDialog(CharacterSlot::HAND_LEFT);
		}
	});

	toggleActive();
}

CharacterControl::~CharacterControl()
{}

void CharacterControl::onGameConfigLoaded(GameDatabase* db, GameFileSystem* fs, ModelSupport& ms)
{
	WidgetUsesGameClient::onGameConfigLoaded(db, fs, ms);

	tabardCustomizationProvider = ms.tabardCustomizationProviderFactory(fs);
	characterCustomizationProvider = ms.characterCustomizationProviderFactory(fs, db);
}

void CharacterControl::onModelChanged(Model* target) {
	model = target;

	customizationSizes.clear();
	chosenCustomisations = model->characterCustomizationChoices;

	characterCustomizationProvider->reset();

	if (gameDB != nullptr && model != nullptr && model->model->isCharacter()) {

		Log::message("Character control enabled.");

		QString modelFileName = model->model->getFileInfo().path;
		auto parts = modelFileName.split(gameFS->seperator());
		if (parts.length() >= 4) {

			const auto& genderName = parts[parts.length() - 2];
			const auto& raceName = parts[parts.length() - 3];

			CharacterDetails info = CharacterDetails();

			info.gender = GenderUtil::fromString(genderName);

			auto charRaceRecord = gameDB->characterRacesDB->find([&](const CharacterRaceRecordAdaptor* item) -> bool {
				auto recordName = item->getClientFileString();
				return recordName.compare(raceName, Qt::CaseInsensitive) == 0;
			});

			if (charRaceRecord != nullptr) {
				info.raceId = charRaceRecord->getId();
				info.isHd = model->model->isHDCharacter();

				characterDetails = info;
				characterCustomizationProvider->initialise(characterDetails.value());

				customizationSizes = characterCustomizationProvider->getAvailableOptions();

				if (model->characterCustomizationChoices.size() == 0) {
					model->characterCustomizationChoices = customizationSizes;
					std::ranges::fill(std::ranges::views::values(model->characterCustomizationChoices), 0);
					chosenCustomisations = model->characterCustomizationChoices;
					applyCustomizations();
				}
			}
			else {
				Log::message("Unable to match character race.");
				characterDetails.reset();
			}
		}
		else {
			Log::message("Unknown character path format."); 
			characterDetails.reset();
		}
	}
	else {
		characterDetails.reset();
	}

	toggleActive();
}

void CharacterControl::toggleActive() {
	isLoadingModel = true;

	bool enabled = false;

	if (model != nullptr) {
		enabled = model->model->isCharacter();
	}

	for (auto& item : controlMap) {
		item.second.button->setDisabled(!enabled);
		item.second.label->setDisabled(!enabled);
	}

	for (auto& item : controlMap) {
		updateEquipmentLabel(item.first);
	}

	ui.pushButtonCustomTabard->setDisabled(!enabled);

	ui.pushButtonRandomise->setDisabled(!enabled);

	ui.comboBoxEyeGlow->setDisabled(!enabled);
	ui.comboBoxEyeGlow->setCurrentIndex(1);

	ui.checkBoxUnderWear->setDisabled(!enabled);
	ui.checkBoxEars->setDisabled(!enabled);
	ui.checkBoxFeet->setDisabled(!enabled);
	ui.checkBoxHair->setDisabled(!enabled);
	ui.checkBoxFacialHair->setDisabled(!enabled);
	ui.checkBoxSheatheWeapons->setDisabled(!enabled);

	ui.pushButtonEffectRight->setDisabled(!enabled);
	ui.pushButtonEffectLeft->setDisabled(!enabled);

	ui.pushButtonMount->setDisabled(!enabled);



	if (model != nullptr && model->model->isCharacter()) {

		ui.comboBoxEyeGlow->setCurrentIndex(model->characterOptions.eyeGlow);
		ui.checkBoxUnderWear->setChecked(model->characterOptions.showUnderWear);
		ui.checkBoxEars->setChecked(model->characterOptions.showEars);
		ui.checkBoxFeet->setChecked(model->characterOptions.showFeet);
		ui.checkBoxHair->setChecked(model->characterOptions.showHair);
		ui.checkBoxFacialHair->setChecked(model->characterOptions.showFacialHair);
		ui.checkBoxSheatheWeapons->setChecked(model->characterOptions.sheatheWeapons);

		for (const auto& custom : customizationSizes) {
			const auto label = QString::fromStdString(custom.first);
			QComboBox* combo = getCustomizationControl(label);
			bool added = false;

			if (combo == nullptr) {
				combo = addCustomizationControl(label);
				added = true;
			}
			else {
				combo->clear();
			}

			for (uint32_t i = 0; i < custom.second; i++) {
				combo->addItem(QString::number(i));
			}
			
			combo->setCurrentIndex(chosenCustomisations.at(custom.first));

			if (added) {
				connect(combo, &QComboBox::currentIndexChanged, [&, name = custom.first](int index) {
					if (model != nullptr && !isLoadingModel) {
						chosenCustomisations[name] = std::clamp(index, 0, static_cast<int32_t>(customizationSizes.at(name) - 1));

						if (!isRandomising) {
							applyCustomizations();
							updateModel();
						}
					}
					});
			}
		}

		//remove rows that are no longer valid
		const auto& known_keys = std::ranges::views::keys(customizationSizes);
		for (auto i = 0; i < ui.formLayoutCustomizations->rowCount(); i++) {
			QLabel* lbl = (QLabel*)ui.formLayoutCustomizations->itemAt(i, QFormLayout::LabelRole)->widget();
			if (std::ranges::count(known_keys, lbl->text().toStdString()) == 0) {
				ui.formLayoutCustomizations->removeRow(i--);
			}
		}	

		assert(customizationSizes.size() == ui.formLayoutCustomizations->rowCount());

		updateModel();
		updateEquipment();
	}
	else {

		//remove all custom options.
		while (ui.formLayoutCustomizations->rowCount() > 0) {
			ui.formLayoutCustomizations->removeRow(0);
		}

		ui.comboBoxEyeGlow->setCurrentIndex(CharacterRenderOptions::EyeGlow::NORMAL);
		ui.checkBoxUnderWear->setChecked(true);
		ui.checkBoxEars->setChecked(true);
		ui.checkBoxFeet->setChecked(true);
		ui.checkBoxHair->setChecked(true);
		ui.checkBoxFacialHair->setChecked(true);
		ui.checkBoxSheatheWeapons->setChecked(true);
	}

	isLoadingModel = false;
}

void CharacterControl::openChoiceDialog(CharacterSlot slot)
{
	auto choiceDialog = new EquipmentChoiceDialog(gameDB, slot, this);
	choiceDialog->setAttribute(Qt::WA_DeleteOnClose);
	connect(choiceDialog, &EquipmentChoiceDialog::chosen, [&](CharacterSlot slot, const ItemRecordAdaptor* item_record) {
		if (model != nullptr) {
			if (item_record != nullptr) {
				model->characterEquipment[slot] = item_record;
			}
			else {
				model->characterEquipment.erase(slot);
			}

			if (slot == CharacterSlot::TABARD) {
				model->tabardCustomization = std::nullopt;
			}

			updateEquipmentLabel(slot);
			if (CharacterUtil::slotHasModel(slot)) {
				updateItem(slot, item_record);
			}
			else {
				updateModel();
			}
		}
		});
	choiceDialog->show();
}

void CharacterControl::updateEquipmentLabel(CharacterSlot slot)
{
	if (model != nullptr && model->characterEquipment.contains(slot)) {
		controlMap[slot].label->setText(model->characterEquipment[slot]->getName());
	}
	else {
		controlMap[slot].label->setText("-- Empty --");
	}
}

void CharacterControl::randomiseComboBox(QComboBox* comboBox)
{
	auto item_count = comboBox->count();
	if (item_count > 0) {
		comboBox->setCurrentIndex(QRandomGenerator::global()->bounded(0, item_count - 1));
	}
}

void CharacterControl::openEnchantDialog(CharacterSlot slot)
{
	auto dialog = new EffectChoiceDialog(gameDB, slot, this);
	dialog->setAttribute(Qt::WA_DeleteOnClose);
	connect(dialog, &EffectChoiceDialog::chosen, [&](CharacterSlot slot, std::optional<uint32_t> enchantId) {

		const auto& model_attachments = model->getAttachments();
		auto attachment = std::find_if(model_attachments.begin(), model_attachments.end(), [slot](const Attachment* attach) -> bool {
			return attach->characterSlot == slot;
		});

		if (attachment == model_attachments.end()) {
			return;
		}

		auto updateLabel = [&,slot](QString text) {
			if (slot == CharacterSlot::HAND_RIGHT) {
				ui.labelEffectRight->setText(text);
			} else if(slot == CharacterSlot::HAND_LEFT) {
				ui.labelEffectLeft->setText(text);
			}
			else {
				assert(false);
			}
		};

		//TODO update label properly, e.g including on model change.

		updateLabel("-- Empty --");

		(*attachment)->effects.clear();

		if (enchantId.has_value()) {
			auto enchant = gameDB->spellEnchantmentsDB->findById(enchantId.value());
			assert(enchant != nullptr);

			updateLabel(enchant->getName());
			
			auto itemVisual = gameDB->itemVisualsDB->findById(enchant->getItemVisualId());
			assert(itemVisual != nullptr);

			applyItemVisualToAttachment(*attachment, itemVisual);

		}
	});
	dialog->show();
}

void CharacterControl::applyCustomizations()
{
	if (!characterCustomizationProvider->apply(model, characterDetails.value(), chosenCustomisations)) {
		Log::message("Character customization invalid after refresh");
	}

}

void CharacterControl::setGeosetVisibility(CharacterGeosets geoset, uint32_t flags)
{
	//formula for converting a geoset flag into an id
	//xx1 id's look to be the default, so +1 gets added to the flags
	auto geoset_id = (geoset * 100) + 1 + flags;

	auto id_range_start = geoset * 100;
	auto id_range_end = (geoset + 1) * 100;

	assert(id_range_start <= geoset_id);
	assert(id_range_end >= geoset_id);

	if (model != nullptr) {
		for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
			auto& record = model->model->getGeosetAdaptors()[i];
			if (record->getId() == geoset_id) {
				model->visibleGeosets[i] = true;
			}
			else if (id_range_start < record->getId() && record->getId() < id_range_end) {
				model->visibleGeosets[i] = false;
			}
		}
	}
}

void CharacterControl::updateModel()
{
	if (model != nullptr && model->characterCustomization.isValid()) {

		for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
			//load all the default geosets
			//e.g 0, 101, 201, 301 ... etc
			//equipment is responsible for unsetting the visibility of the default geosets.
			auto geoset_id = model->model->getGeosetAdaptors()[i]->getId();
			model->visibleGeosets[i] = geoset_id == 0 || (geoset_id > 100 && geoset_id % 100 == 1);
		}

		//TODO handle eye glow

		std::shared_ptr<Texture> hairtex = nullptr;
		std::shared_ptr<Texture> capetex = nullptr;
		std::shared_ptr<Texture> furtex = nullptr;

		CharacterTextureBuilder builder;

		{
			const auto& skin = model->characterCustomization.skin->getTextures();

			if (!skin[0].isEmpty()) {
				builder.setBaseLayer(skin[0]);
			}

			if (!skin[1].isEmpty()) {
				furtex = scene->textureManager.add(skin[1], gameFS);
			}

			if (model->characterOptions.showUnderWear) {
				auto record = gameDB->characterSectionsDB->find([&](const CharacterSectionRecordAdaptor* adaptor) ->bool {
					return adaptor->getRaceId() == characterDetails.value().raceId &&
					adaptor->getSexId() == characterDetails.value().gender &&
					adaptor->getVariationIndex() == model->characterCustomization.skin->getVariationIndex() &&
					adaptor->isHD() == characterDetails.value().isHd &&
					adaptor->getType() == CharacterSectionType::Underwear;
				});

				if (record != nullptr) {
					const auto& underwear_skins = record->getTextures();
					if (!underwear_skins[0].isEmpty()) {
						builder.addLayer(underwear_skins[0], CharacterRegion::LEG_UPPER, 1);
					}
					if (!underwear_skins[1].isEmpty()) {
						builder.addLayer(underwear_skins[1], CharacterRegion::TORSO_UPPER, 1);
					}
				}
			}
		}

		auto& hairStyle = model->characterCustomization.hairStyle;

		const auto hair_geoset_id = std::max(1u, hairStyle->getGeoset());
		for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
			if (model->model->getGeosetAdaptors()[i]->getId() == hair_geoset_id) {
				model->visibleGeosets[i] = model->characterOptions.showHair;
			}
		}

		{
			const auto& face = model->characterCustomization.face->getTextures();

			if (!face[0].isEmpty()) {
				builder.addLayer(face[0], CharacterRegion::FACE_LOWER, 1);
			}

			if (!face[1].isEmpty()) {
				builder.addLayer(face[1], CharacterRegion::FACE_UPPER, 1);
			}
		}


		if (model->characterOptions.showFacialHair) {
			const auto& facial_geoset = model->characterCustomization.facialStyle;

			//must be atleast 1, can be problematic if it doesnt get shown at all.
			//NOTE records ate in 100, 300, 200 order
			//TODO check logic, is the adaptor returing data in incorrect order?
			setGeosetVisibility(CharacterGeosets::CG_GEOSET100, facial_geoset->getGeoset100());
			setGeosetVisibility(CharacterGeosets::CG_GEOSET200, facial_geoset->getGeoset300());
			setGeosetVisibility(CharacterGeosets::CG_GEOSET300, facial_geoset->getGeoset200());

			if (model->characterCustomization.facialColour != nullptr)
			{
				const auto& face_feature = model->characterCustomization.facialColour->getTextures();

				if (!face_feature[0].isEmpty()) {
					builder.addLayer(face_feature[0], CharacterRegion::FACE_LOWER, 2);
				}

				if (!face_feature[1].isEmpty()) {
					builder.addLayer(face_feature[1], CharacterRegion::FACE_UPPER, 2);
				}
			}
		}

		{
			const auto& hair = model->characterCustomization.hairColour->getTextures();
			
			if (!hair[0].isEmpty()) {
				hairtex = scene->textureManager.add(hair[0], gameFS);
			}

			if (!hairStyle->isBald())
			{
				if (!hair[1].isEmpty()) {
					builder.addLayer(hair[1], CharacterRegion::FACE_LOWER, 3);
				}


				if (!hair[2].isEmpty()) {
					builder.addLayer(hair[2], CharacterRegion::FACE_UPPER, 3);
				}
			}

			if (hairtex == nullptr) {
				//TODO need to use alternative texture?
			}
		}

		if (model->characterOptions.showEars) {
			setGeosetVisibility(CharacterGeosets::CG_EARS, 1);
		}

		for (auto i = 0; i < (uint32_t)CharacterSlot::MAX; i++) {
			CharacterSlot slot = (CharacterSlot)i;
			auto layer_index = 10 + i; 

			if (model->characterEquipment.contains(slot)) {
				const auto& item_record = model->characterEquipment[slot];

				if (item_record == nullptr) {
					continue;
				}

				const auto* record = gameDB->itemDisplayDB->findById(item_record->getItemDisplayInfoId());

				if(record != nullptr) {
					switch (slot) {
					case CharacterSlot::CHEST:
					case CharacterSlot::SHIRT:
					{
						setGeosetVisibility(CharacterGeosets::CG_WRISTBANDS, record->getGeosetGlovesFlags());

						auto arm1_skin = record->getTextureUpperArm();
						auto arm2_skin = record->getTextureLowerArm();
						auto chest1_skin = record->getTextureUpperChest();
						auto chest2_skin = record->getTextureLowerChest();

						if (!arm1_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(arm1_skin, CharacterRegion::ARM_UPPER),
								CharacterRegion::ARM_UPPER,
								layer_index
							);
						}

						if (!arm2_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(arm2_skin, CharacterRegion::ARM_LOWER),
								CharacterRegion::ARM_LOWER,
								layer_index
							);
						}

						if (!chest1_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(chest1_skin, CharacterRegion::TORSO_UPPER),
								CharacterRegion::TORSO_UPPER,
								layer_index
							);
						}

						if (!chest2_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(chest2_skin, CharacterRegion::TORSO_LOWER),
								CharacterRegion::TORSO_LOWER,
								layer_index
							);
						}

						if (item_record->getInventorySlotId() == ItemInventorySlotId::ROBE || record->getGeosetRobeFlags() == 1) {
							auto leg1_skin = record->getTextureUpperLeg();
							auto leg2_skin = record->getTextureLowerLeg();
								
							if (!leg1_skin.isEmpty()) {
								builder.addLayer(
									searchSlotTexture(leg1_skin, CharacterRegion::LEG_UPPER),
									CharacterRegion::LEG_UPPER,
									layer_index
								);
							}

							if (!leg2_skin.isEmpty()) {
								builder.addLayer(
									searchSlotTexture(leg2_skin, CharacterRegion::LEG_LOWER),
									CharacterRegion::LEG_LOWER,
									layer_index
								);
							}

							setGeosetVisibility(CharacterGeosets::CG_TROUSERS, record->getGeosetRobeFlags());

							//TODO try hide boots and tabard
						}	
					}
					break;
					case CharacterSlot::BELT:
					{
						auto chest_lower_texture = record->getTextureLowerChest();
						if (!chest_lower_texture.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(chest_lower_texture, CharacterRegion::TORSO_LOWER),
								CharacterRegion::TORSO_LOWER,
								layer_index
							);
						}

						auto legs_upper_texture = record->getTextureUpperLeg();
						if (!legs_upper_texture.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(legs_upper_texture, CharacterRegion::LEG_UPPER),
								CharacterRegion::LEG_UPPER,
								layer_index
							);
						}
					}
					break;
					case CharacterSlot::BRACERS:
					{
						auto bracer_texture = record->getTextureLowerArm();
						if (!bracer_texture.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(bracer_texture, CharacterRegion::ARM_LOWER),
								CharacterRegion::ARM_LOWER,
								layer_index
							);
						}
					}
					break;
					case CharacterSlot::PANTS:
					{
						setGeosetVisibility(CharacterGeosets::CG_KNEEPADS, record->getGeosetBracerFlags());

						auto leg_upper_skin = record->getTextureUpperLeg();
						if (!leg_upper_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(leg_upper_skin, CharacterRegion::LEG_UPPER),
								CharacterRegion::LEG_UPPER,
								layer_index
							);
						}

						auto leg_lower_skin = record->getTextureLowerLeg();
						if (!leg_lower_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(leg_lower_skin, CharacterRegion::LEG_LOWER),
								CharacterRegion::LEG_LOWER,
								layer_index
							);
						}
					}
					break;
					case CharacterSlot::GLOVES:
					{
						setGeosetVisibility(CharacterGeosets::CG_GLOVES, record->getGeosetGlovesFlags());

						auto hands_skin = record->getTextureHands();

						if (!hands_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(hands_skin, CharacterRegion::HAND),
								CharacterRegion::HAND,
								layer_index
							);
						}

						auto arm_lower_skin = record->getTextureLowerArm();

						if (!hands_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(arm_lower_skin, CharacterRegion::ARM_LOWER),
								CharacterRegion::ARM_LOWER,
								layer_index
							);
						}

					}
					break;
					case CharacterSlot::BOOTS:
					{
						setGeosetVisibility(CharacterGeosets::CG_BOOTS, record->getGeosetGlovesFlags());

						auto lower_leg_skin = record->getTextureLowerLeg();

						if (!lower_leg_skin.isEmpty()) {
							builder.addLayer(
								searchSlotTexture(lower_leg_skin, CharacterRegion::LEG_LOWER),
								CharacterRegion::LEG_LOWER,
								layer_index
							);
						}

						if (model->characterOptions.showFeet) {
							auto foot_skin = record->getTextureFoot();
							if (!foot_skin.isEmpty()) {
								builder.addLayer(
									searchSlotTexture(foot_skin, CharacterRegion::FOOT),
									CharacterRegion::FOOT,
									layer_index
								);
							}
						}
					}
					break;
					case CharacterSlot::TABARD:
					{
						setGeosetVisibility(CharacterGeosets::CG_TABARD, 1);

						if (model->tabardCustomization.has_value()) {
							const auto& tabard_upper_texs = model->tabardCustomization.value().texturesUpperChest;
							const auto& tabard_lower_texs = model->tabardCustomization.value().texturesLowerChest;

							for (const auto& tabard_upper : tabard_upper_texs) {
								if (!tabard_upper.isEmpty()) {
									builder.addLayer(
										tabard_upper,
										CharacterRegion::TORSO_UPPER,
										layer_index
									);
								}
							}

							for (const auto& tabard_lower : tabard_lower_texs) {
								if (!tabard_lower.isEmpty()) {
									builder.addLayer(
										tabard_lower,
										CharacterRegion::TORSO_LOWER,
										layer_index
									);
								}
							}
						}
						else {
							auto tabard_upper = record->getTextureUpperChest();
							auto tabard_lower = record->getTextureLowerChest();

							if (!tabard_upper.isEmpty()) {
								builder.addLayer(
									searchSlotTexture(tabard_upper, CharacterRegion::TORSO_UPPER),
									CharacterRegion::TORSO_UPPER,
									layer_index
								);
							}


							if (!tabard_lower.isEmpty()) {
								builder.addLayer(
									searchSlotTexture(tabard_lower, CharacterRegion::TORSO_LOWER),
									CharacterRegion::TORSO_LOWER,
									layer_index
								);
							}
						}
					}
					break;
					case CharacterSlot::CAPE:
					{
						setGeosetVisibility(CharacterGeosets::CG_CAPE, record->getGeosetGlovesFlags());

						auto cape_skin = record->getModelTexture(CharacterSlot::CAPE, ItemInventorySlotId::CAPE)[0];
						if (!cape_skin.isEmpty()) {
							//TODO use 'searchSlotTexture' ?
							capetex = scene->textureManager.add(cape_skin, gameFS);
						}
					}
					break;
					}
				} else {
					Log::message("Unable to find item display - itemId: " + QString::number(item_record->getId()));
				}
				
			}
		}

		CharacterComponentTextureAdaptor* componentTextureAdaptor = &legacyComponentTextureAdaptor;

		if (gameDB->characterComponentTexturesDB != nullptr && characterDetails.has_value()) {
			auto raceInfo = gameDB->characterRacesDB->findById(characterDetails.value().raceId);
			const bool is_hd_model = characterDetails.value().isHd;

			if (raceInfo != nullptr && raceInfo->getComponentTextureLayoutId(is_hd_model).has_value()) {
				const auto raceLayoutId = raceInfo->getComponentTextureLayoutId(is_hd_model).value();
				auto temp_componentAdaptor = gameDB->characterComponentTexturesDB->find([raceLayoutId](const CharacterComponentTextureAdaptor* componentAdaptor) -> bool {
					return componentAdaptor->getLayoutId() == raceLayoutId;
				});

				if (temp_componentAdaptor != nullptr) {
					componentTextureAdaptor = const_cast<CharacterComponentTextureAdaptor*>(temp_componentAdaptor);
				}
			}
		}

		model->replacableTextures[TextureType::BODY] = builder.build(componentTextureAdaptor, & scene->textureManager, gameFS);

		if (hairtex != nullptr) {
			model->replacableTextures[TextureType::HAIR] = hairtex;
		}
		else {
			model->replacableTextures.erase(TextureType::HAIR);
		}

		if (capetex != nullptr) {
			model->replacableTextures[TextureType::CAPE] = capetex;
		}
		else {
			model->replacableTextures.erase(TextureType::CAPE);
		}

		if (furtex != nullptr) {
			model->replacableTextures[TextureType::FUR] = furtex;
		}
		else {
			model->replacableTextures.erase(TextureType::FUR);
		}

		//TODO GAMEOBJECT1

		//TODO geosets	
	}
}

void CharacterControl::updateEquipment()
{
	for (const auto& equipment : model->characterEquipment) {
		if (CharacterUtil::slotHasModel(equipment.first)) {
			updateItem(equipment.first, equipment.second);
		}
	}
}

std::vector<AttachmentPosition> CharacterControl::getAttachmentPositions(CharacterSlot slot, const ItemRecordAdaptor* item) {
	std::vector<AttachmentPosition> attach_positions;

	switch (slot) {
	case CharacterSlot::HEAD:
		attach_positions = { AttachmentPosition::HELMET };
		break;
	case CharacterSlot::SHOULDER:
		attach_positions = {
			AttachmentPosition::LEFT_SHOULDER,
			AttachmentPosition::RIGHT_SHOULDER,
		};
		break;
	case CharacterSlot::HAND_LEFT:
		attach_positions = { AttachmentPosition::LEFT_PALM };
		break;
	case CharacterSlot::HAND_RIGHT:
		attach_positions = { AttachmentPosition::RIGHT_PALM };
		break;
	case CharacterSlot::QUIVER:
		attach_positions = { AttachmentPosition::RIGHT_BACK_SHEATH };
		break;
	}

	if (slot == CharacterSlot::HAND_LEFT || slot == CharacterSlot::HAND_RIGHT) {
		if (item->getInventorySlotId() == ItemInventorySlotId::SHIELD) {
			attach_positions = { AttachmentPosition::LEFT_WRIST };
		}

		auto sheath_type = (core::SheathTypes)item->getSheatheTypeId();
		if (model->characterOptions.sheatheWeapons && sheath_type > SheathTypes::SHEATHETYPE_NONE) {
			attach_positions = {
				core::Mapping::sheathTypeAttachmentPosition(sheath_type, slot)
			};
		}
	}

	return attach_positions;
}

void CharacterControl::updateItem(CharacterSlot slot, const ItemRecordAdaptor* item)
{
	if (item == nullptr) {
		model->removeAttachments(slot);
		return;
	}

	std::vector<AttachmentPosition> attach_positions = getAttachmentPositions(slot, item);

	auto item_display = gameDB->itemDisplayDB->findById(item->getItemDisplayInfoId());
	if (item_display == nullptr) {
		Log::message(QString("Unable to find item display for slot: %1").arg((int32_t)slot));
		return;
	}

	size_t attachments_added = 0;

	auto attachment_index = 0;
	for (auto attach_pos : attach_positions) {
		GameFileUri model_path = item_display->getModel(slot, item->getInventorySlotId())[attachment_index];


		if (model_path.isPath()) {

			if (slot == CharacterSlot::HEAD) {
				if (model_path.isPath()) {
					//head handling include race / gender data, seems to only be needed for path types.
					auto model_file_name = GameFileUri::removeExtension(model_path.getPath());
					auto race_record = gameDB->characterRacesDB->findById(characterDetails.value().raceId);
					model_file_name.append("_");
					if (race_record != nullptr) {
						model_file_name.append(race_record->getClientPrefix());
					}
					model_file_name.append(GenderUtil::toChar(characterDetails.value().gender));
					model_file_name.append(".m2");
					model_path = model_file_name;
				}
			}
			else {
				model_path = GameFileUri::replaceExtension(model_path.getPath(), "m2");
			}
		}

		try {

			auto att = std::make_unique<Attachment>(modelSupport.modelFactory);
			att->attachmentPosition = attach_pos;
			att->characterSlot = slot;

			auto loadTexture = std::bind(&ModelTextureInfo::loadTexture,
				att.get(),
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4,
				std::ref(scene->textureManager),
				gameFS
			);
			att->model->load(gameFS, model_path, loadTexture);
			att->initAnimationData(att->model.get());

			//load attachment texture
			GameFileUri texture_file_name = item_display->getModelTexture(slot, item->getInventorySlotId())[attachment_index];
			Log::message("Loaded attachment texture: " + texture_file_name.toString());
			auto tex = scene->textureManager.add(texture_file_name, gameFS);
			if (tex != nullptr) {
				att->replacableTextures[TextureType::CAPE] = tex;
			}

			model->setAttachmentPosition(att.get(), attach_pos);


			//TODO HANDLE ITEM VISUALS! (BFA)
			/*auto itemVisualId = item_display->getItemVisualId();

			if (itemVisualId > 0) {
				auto itemVisual = gameDB->itemVisualsDB->findById(itemVisualId);

				assert(itemVisual != nullptr);

				applyItemVisualToAttachment(att.get(), itemVisual);
			}*/

			model->addAttachment(std::move(att));

			attachments_added++;
		}
		catch (std::exception e) {
			Log::message(QString("Exception caught loading attachment %1:").arg(attachment_index));
			Log::message(e.what());
			QMessageBox::warning(this, "Attachment Data Error", "An error occured while loading model data.", QMessageBox::Ok);
		}

		attachment_index++;
	}

	if (attachments_added != attach_positions.size()) {
		assert(false);
		//TODO force remove equipment if it cant be loaded safely.
	}
}

GameFileUri CharacterControl::searchSlotTexture(GameFileUri file, CharacterRegion region)
{
	if (file.isId()) {
		return file;
	}
	
	QString name = file.getPath();
	const core::Gender charGender = characterDetails.value().gender;

	static const std::map<CharacterRegion,QString> regionPaths =
	{
		{ CharacterRegion::ARM_UPPER, QString("Item\\TextureComponents\\ArmUpperTexture\\") },
		{ CharacterRegion::ARM_LOWER, QString("Item\\TextureComponents\\ArmLowerTexture\\") },
		{ CharacterRegion::HAND, QString("Item\\TextureComponents\\HandTexture\\") },
		{ CharacterRegion::TORSO_UPPER, QString("Item\\TextureComponents\\TorsoUpperTexture\\") },
		{ CharacterRegion::TORSO_LOWER, QString("Item\\TextureComponents\\TorsoLowerTexture\\") },
		{ CharacterRegion::LEG_UPPER, QString("Item\\TextureComponents\\LegUpperTexture\\") },
		{ CharacterRegion::LEG_LOWER, QString("Item\\TextureComponents\\LegLowerTexture\\") },
		{ CharacterRegion::FOOT, QString("Item\\TextureComponents\\FootTexture\\") }
	};


	if (name.length() < 3) {
		return "";
	}

	QChar leggings = name[name.length() - 2];

	QString fn = regionPaths.at(region);

	fn += name;
	fn += "_";

	if (leggings == 'l' || leggings == 'L') {
		fn += "U";
	}
	else {
		fn += GenderUtil::toChar(charGender);
	}

	fn += ".blp";

	auto temp_file = gameFS->openFile(fn);
	if (temp_file != nullptr) {
		gameFS->closeFile(temp_file);
		return fn;
	}

	if (fn.length() < 5) {
		return "";
	}

	if (leggings == 'l' || leggings == 'L') {
		fn[fn.length() - 5] = GenderUtil::toChar(charGender);
	}
	else {
		fn[fn.length() - 5] = 'U';
	}

	auto temp_file2 = gameFS->openFile(fn);
	if (temp_file2 != nullptr) {
		gameFS->closeFile(temp_file2);
		return fn;
	}

	fn = regionPaths.at(region);
	fn += name;
	fn += ".blp";

	return fn;
}

void CharacterControl::applyItemVisualToAttachment(Attachment* attachment, const ItemVisualRecordAdaptor* itemVisual)
{
	assert(itemVisual != nullptr);

	auto visualEffectIds = itemVisual->getItemVisualEffectIds();

	auto itemVisualEffects = gameDB->itemVisualEffectsDB->where([visualEffectIds](const ItemVisualEffectRecordAdaptor* adaptor) {
		return std::find(visualEffectIds.begin(), visualEffectIds.end(), adaptor->getId()) != visualEffectIds.end();
	});

	for (auto& effect : itemVisualEffects) {
		if (!effect->getModel().isEmpty()) {
			auto model_str = GameFileUri::replaceExtension(effect->getModel(), "mdx", "m2");
			auto m = std::make_unique<Attachment::Effect>(modelSupport.modelFactory);

			auto loadTexture = std::bind(&ModelTextureInfo::loadTexture,
				m.get(),
				std::placeholders::_1,
				std::placeholders::_2,
				std::placeholders::_3,
				std::placeholders::_4,
				std::ref(scene->textureManager),
				gameFS
			);

			m->model->load((MPQFileSystem*)gameFS, model_str, loadTexture);
			m->initAnimationData(m->model.get());
			attachment->effects.push_back(std::move(m));
		}
	}
}

QComboBox* CharacterControl::addCustomizationControl(const QString& name) {

	QComboBox* combobox = new QComboBox(this);
	combobox->setEditable(false);

	ui.formLayoutCustomizations->addRow(name, combobox);
	
	return combobox;
}

QComboBox* CharacterControl::getCustomizationControl(const QString& name) {
	
	for (auto i = 0; i < ui.formLayoutCustomizations->rowCount(); i++) {
		QLabel* lbl = (QLabel*)ui.formLayoutCustomizations->itemAt(i, QFormLayout::LabelRole)->widget();
		if (lbl->text() == name) {
			return (QComboBox*)ui.formLayoutCustomizations->itemAt(i, QFormLayout::FieldRole)->widget();
		}
	}

	return nullptr;
}
