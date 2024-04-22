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

		std::optional<CharacterItemWrapper> tabard_item;

		if (model->characterEquipment.contains(core::CharacterSlot::TABARD)) {
			tabard_item = model->characterEquipment.at(core::CharacterSlot::TABARD);
		}

		auto customDialog = new CustomTabardDialog(gameDB, 
		gameFS, 
		tabardCustomizationProvider.get(),
		tabard_item, 
		model->tabardCustomizationChoices,
		this);
		customDialog->setAttribute(Qt::WA_DeleteOnClose);
		connect(customDialog, &CustomTabardDialog::chosen, [&](DialogChoiceMethod method,
			std::optional<core::CharacterItemWrapper> wrapper,
			std::optional<core::TabardCustomizationOptions> options) {
	
#ifdef _DEBUG
			if (options.has_value()) {
				assert(wrapper.has_value());
			}
#endif

			model->tabardCustomizationChoices = options;

			if (wrapper.has_value()) {
				model->characterEquipment.insert_or_assign(core::CharacterSlot::TABARD, wrapper.value());
			}
			else {
				model->characterEquipment.erase(core::CharacterSlot::TABARD);
			}
			
			if (method != DialogChoiceMethod::PREVIEW) {
				updateEquipmentLabel(core::CharacterSlot::TABARD);
			}
			updateModel();
		});
		customDialog->show();
	});

	connect(ui.pushButtonRandomise, &QPushButton::pressed, [&]() {
		if (model != nullptr && !isLoadingModel) {
			isRandomising = true;
			for (const auto& custom : availableCustomizations) {
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
				const auto slot = attachment->getSlot();
				if ((slot == CharacterSlot::HAND_LEFT ||
					slot == CharacterSlot::HAND_RIGHT) &&
					model->characterEquipment.contains(slot)) {

					const auto& item_wrapper = model->characterEquipment.at(slot);
					const auto positions = attachmentCustomizationProvider->getAttachmentPositions(slot, item_wrapper.item(), model->characterOptions.sheatheWeapons);
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
	attachmentCustomizationProvider = ms.attachmentCustomizationProviderFactory(fs, db);
}

void CharacterControl::onSceneLoaded(core::Scene* new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::modelSelectionChanged, this, &CharacterControl::onModelChanged);
}


void CharacterControl::onModelChanged(Model* target) {
	model = target;

	availableCustomizations.clear();
	if (model != nullptr) {
		chosenCustomisations = model->characterCustomizationChoices;
	}
	else {
		chosenCustomisations.clear();
	}

	characterCustomizationProvider->reset();

	bool searchContextFound = false;

	if (gameDB != nullptr && model != nullptr && model->model->isCharacter()) {
		const auto& char_details = model->getCharacterDetails();
		if (char_details.has_value()) {
			Log::message("Character control enabled.");

			if (char_details.has_value()) {
				const auto* race_adaptor = gameDB->characterRacesDB->findById(char_details->raceId);

				if (race_adaptor != nullptr) {
					modelSearchContext = race_adaptor->getModelSearchContext(char_details->gender);
					textureSearchContext = race_adaptor->getTextureSearchContext(char_details->gender);
					searchContextFound = true;
				}
			}

			characterCustomizationProvider->initialise(char_details.value());
			availableCustomizations = characterCustomizationProvider->getAvailableOptions();

			if (model->characterCustomizationChoices.size() == 0) {
				for (const auto& available_option : availableCustomizations) {
					model->characterCustomizationChoices[available_option.first] = 0;
				}
				chosenCustomisations = model->characterCustomizationChoices;
			}

			applyCustomizations();
		}
	}

	if (!searchContextFound) {
		modelSearchContext = std::nullopt;
		textureSearchContext = std::nullopt;
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

		for (const auto& custom : availableCustomizations) {
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

			for (const auto& choice : custom.second) {
				combo->addItem(QString::fromStdString(choice));
			}

			combo->setCurrentIndex(chosenCustomisations.at(custom.first));

			if (added) {
				connect(combo, &QComboBox::currentIndexChanged, [&, name = custom.first](int index) {
					if (model != nullptr && !isLoadingModel) {
						chosenCustomisations[name] = std::clamp(index, 0, static_cast<int32_t>(availableCustomizations.at(name).size() - 1));

						if (!isRandomising) {
							applyCustomizations();
							updateModel();
						}
					}
					});
			}
		}

		//remove rows that are no longer valid
		const auto& known_keys = std::ranges::views::keys(availableCustomizations);
		for (auto i = 0; i < ui.formLayoutCustomizations->rowCount(); i++) {
			QLabel* lbl = (QLabel*)ui.formLayoutCustomizations->itemAt(i, QFormLayout::LabelRole)->widget();
			if (std::ranges::count(known_keys, lbl->text().toStdString()) == 0) {
				ui.formLayoutCustomizations->removeRow(i--);
			}
		}

		assert(availableCustomizations.size() == ui.formLayoutCustomizations->rowCount());

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

	std::optional<CharacterItemWrapper> existing_item = std::nullopt;

	if(model->characterEquipment.contains(slot)) {
		existing_item = model->characterEquipment.at(slot);
	}

	auto choiceDialog = new EquipmentChoiceDialog(gameDB, slot, existing_item, this);
	choiceDialog->setAttribute(Qt::WA_DeleteOnClose);

	connect(choiceDialog, &EquipmentChoiceDialog::chosen, [&](DialogChoiceMethod method, CharacterSlot slot, std::optional<core::CharacterItemWrapper> wrapper) {
		if (model != nullptr) {
			if (wrapper.has_value()) {
				model->characterEquipment.insert_or_assign(slot, wrapper.value());
			}
			else {
				model->characterEquipment.erase(slot);
			}

			if (method == DialogChoiceMethod::NEW) {
				if (slot == CharacterSlot::TABARD) {
					model->tabardCustomizationChoices = std::nullopt;
				}
			}

			if (method != DialogChoiceMethod::PREVIEW) {
				updateEquipmentLabel(slot);
			}
			

			if (wrapper.has_value()) {
				updateItem(slot, model->characterEquipment[slot]);
			} else {
				model->removeAttachments(slot);
			}

			updateModel();
		}
		});
	choiceDialog->show();
}

void CharacterControl::updateEquipmentLabel(CharacterSlot slot)
{
	if (model != nullptr && model->characterEquipment.contains(slot)) {
		controlMap[slot].label->setText(model->characterEquipment[slot].item()->getName());
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
			return attach->getSlot() == slot;
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
	if (!characterCustomizationProvider->apply(model, model->getCharacterDetails().value(), chosenCustomisations)) {
		Log::message("Character customization invalid after refresh");
	}
}

void CharacterControl::updateModel()
{
	if (model != nullptr) {

		Log::message("Updating character model...");

		ModelTraits traits = ModelTraits(model);

		for (auto i = 0; i < model->model->getGeosetAdaptors().size(); i++) {
			//load all the default geosets
			//e.g 0, 101, 201, 301 ... etc
			//equipment is responsible for unsetting the visibility of the default geosets.
			const auto geoset_id = model->model->getGeosetAdaptors()[i]->getId();
			model->forceGeosetVisibilityByIndex(i, geoset_id == 0 || (geoset_id > 100 && geoset_id % 100 == 1));
		}

		std::shared_ptr<Texture> capetex = nullptr;

		CharacterTextureBuilder builder;
		characterCustomizationProvider->update(model, &builder, scene);

		if (!model->characterOptions.showEars) {
			model->clearGeosetVisibility(CharacterGeosets::CG_EARS);
		}
		else if (!model->isGeosetVisible(CharacterGeosets::CG_EARS)) {
			model->setGeosetVisibility(CharacterGeosets::CG_EARS, 1);
		}

		const auto slot_order = getSlotOrder(traits);
		for (auto i = 0; i < (uint32_t)CharacterSlot::MAX; i++) {

			const CharacterSlot slot = slot_order[i];
			auto layer_index = 10 + i; 

			if (model->characterEquipment.contains(slot)) {
				const auto& item_wrapper = model->characterEquipment[slot];
				const auto* record = item_wrapper.display();

				
				switch (slot) {
				case CharacterSlot::CHEST:
				case CharacterSlot::SHIRT:
				{
					model->setGeosetVisibility(CharacterGeosets::CG_WRISTBANDS, record->getGeosetGlovesFlags());

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

					
					if (item_wrapper.item()->getInventorySlotId() == ItemInventorySlotId::ROBE || record->getGeosetRobeFlags() == 1) {
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

						model->setGeosetVisibility(CharacterGeosets::CG_TROUSERS, record->getGeosetRobeFlags());
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
					model->setGeosetVisibility(CharacterGeosets::CG_KNEEPADS, record->getGeosetBracerFlags());
					model->setGeosetVisibility(CharacterGeosets::CG_TROUSERS, record->getGeosetRobeFlags());

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
					model->setGeosetVisibility(CharacterGeosets::CG_GLOVES, record->getGeosetGlovesFlags());

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
					if (!traits.hasRobeBottom) {
						model->setGeosetVisibility(CharacterGeosets::CG_BOOTS, record->getGeosetGlovesFlags());
					}

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
					if (!traits.hasRobeBottom) {
						model->setGeosetVisibility(CharacterGeosets::CG_TABARD, 1);
					}

					if (isCustomTabardEquiped() && model->tabardCustomizationChoices.has_value()) {
						const auto tabard_data = tabardCustomizationProvider->getData(model->tabardCustomizationChoices.value());
						const auto& tabard_upper_texs = tabard_data.texturesUpperChest;
						const auto& tabard_lower_texs = tabard_data.texturesLowerChest;

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
					model->setGeosetVisibility(CharacterGeosets::CG_CAPE, record->getGeosetGlovesFlags());

					const auto cape_skin = record->getModelTexture(CharacterSlot::CAPE, ItemInventorySlotId::CAPE, textureSearchContext)[0];
					if (!cape_skin.isEmpty()) {
						std::visit([&](auto& var) {
							if constexpr (std::is_same_v<GameFileUri::path_t&, decltype(var)>) {
								var = "Item\\ObjectComponents\\Cape\\" + var;
							}
						}, cape_skin);

						capetex = scene->textureManager.add(cape_skin, gameFS);
					}
				}
				break;
				}
				
			}
		}

		CharacterComponentTextureAdaptor* componentTextureAdaptor = &legacyComponentTextureAdaptor;

		if (gameDB->characterComponentTexturesDB != nullptr && model->getCharacterDetails().has_value()) {

			CharacterComponentTextureAdaptor* impl_adaptor = characterCustomizationProvider->getComponentTextureAdaptor(model->getCharacterDetails().value());
			if (impl_adaptor != nullptr) {
				componentTextureAdaptor = impl_adaptor;
			}			
		}

		model->replacableTextures[TextureType::BODY] = builder.build(componentTextureAdaptor, & scene->textureManager, gameFS);

		if (capetex != nullptr) {
			model->replacableTextures[TextureType::CAPE] = capetex;
		}
		else {
			model->replacableTextures.erase(TextureType::CAPE);
		}
	}
}

void CharacterControl::updateEquipment()
{
	for (const auto& equipment : model->characterEquipment) {
		updateItem(equipment.first, equipment.second);
	}
}

void CharacterControl::updateItem(CharacterSlot slot, const core::CharacterItemWrapper& wrapper)
{
	//TODO only update if needed - currenlty item gets removed and re-added even if no changes are needed.
	model->removeAttachments(slot);

	const std::vector<AttachmentPosition> attach_positions = attachmentCustomizationProvider->getAttachmentPositions(slot, wrapper.item(), model->characterOptions.sheatheWeapons);

	const auto* item_display = wrapper.display();
	const auto& char_details = model->getCharacterDetails();

	auto attachment_index = 0;
	const auto item_models = item_display->getModel(slot, wrapper.item()->getInventorySlotId(), modelSearchContext);
	const auto item_textures = item_display->getModelTexture(slot, wrapper.item()->getInventorySlotId(), textureSearchContext);
	assert(item_models.size() >= attach_positions.size());
	assert(item_textures.size() >= attach_positions.size());

	for (auto attach_pos : attach_positions) {

		GameFileUri model_path = item_models[attachment_index];
		GameFileUri texture_path = item_textures[attachment_index];

		if (model_path.isEmpty()) {
			continue;
		}

		if (model_path.isPath()) {
			model_path = GameFileUri::replaceExtension(model_path.getPath(), "m2");
		}

		try {
			
			Log::message("Loaded attachment model: " + model_path.toString());
			Log::message("Loaded attachment texture: " + texture_path.toString());

			std::unique_ptr<Attachment> att = attachmentCustomizationProvider->makeAttachment(
				slot, 
				attach_pos,
				wrapper,
				model_path,
				texture_path,
				model,
				scene
			);

			//TODO move into attachment provider.
			//TODO handle item visuals for BFA+
			const auto itemVisualId = item_display->getItemVisualId();

			if (itemVisualId > 0 && gameDB->itemVisualsDB != nullptr) {
				const auto* itemVisual = gameDB->itemVisualsDB->findById(itemVisualId);
				assert(itemVisual != nullptr);

				applyItemVisualToAttachment(att.get(), itemVisual);
			}

			model->addAttachment(std::move(att));
		}
		catch (std::exception e) {
			Log::message(QString("Exception caught loading attachment %1:").arg(attachment_index));
			Log::message(e.what());
			QMessageBox::warning(this, "Attachment Data Error", "An error occured while loading model data.", QMessageBox::Ok);
		}

		attachment_index++;
	}
}

GameFileUri CharacterControl::searchSlotTexture(GameFileUri file, CharacterRegion region)
{
	if (file.isId()) {
		return file;
	}
	
	QString name = file.getPath();
	const core::Gender charGender = model->getCharacterDetails()->gender;

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
			auto m = std::make_unique<Attachment::Effect>(modelSupport.modelFactory());

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

bool CharacterControl::isCustomTabardEquiped() const {

	bool customizable_tabard_equiped = false;

	if (model != nullptr) {
		auto possible_tabard_ids = std::views::keys(tabardCustomizationProvider->getTieredCustomTabardItemIds());

		if (model->characterEquipment.contains(core::CharacterSlot::TABARD)) {
			auto current_tabard_item_id = model->characterEquipment[core::CharacterSlot::TABARD].item()->getId();
			customizable_tabard_equiped = std::any_of(possible_tabard_ids.begin(), possible_tabard_ids.end(),
				[&current_tabard_item_id](uint32_t possible) {
					return possible == current_tabard_item_id;
				});

		}
	}

	return customizable_tabard_equiped;
}

CharacterControl::ModelTraits::ModelTraits(core::Model* model) {
	hasRobeBottom = false;

	if (model != nullptr) {

		if (!hasRobeBottom && model->characterEquipment.contains(CharacterSlot::CHEST)) {
			const auto& item_wrapper = model->characterEquipment[CharacterSlot::CHEST];
			const auto* record = item_wrapper.display();
			hasRobeBottom = item_wrapper.item()->getInventorySlotId() == ItemInventorySlotId::ROBE || record->getGeosetRobeFlags() == 1;
		}

		if (!hasRobeBottom && model->characterEquipment.contains(CharacterSlot::PANTS)) {
			const auto& item_wrapper = model->characterEquipment[CharacterSlot::PANTS];
			const auto* record = item_wrapper.display();
			hasRobeBottom = record->getGeosetRobeFlags() == 1;
		}
	}
}

std::vector<core::CharacterSlot> CharacterControl::getSlotOrder(const CharacterControl::ModelTraits& traits) const {
	std::vector<core::CharacterSlot> order{
			CharacterSlot::HEAD,
			CharacterSlot::NECK,
			CharacterSlot::QUIVER,
			CharacterSlot::SHOULDER,
			CharacterSlot::PANTS,
			CharacterSlot::SHIRT,
			CharacterSlot::CHEST,
			CharacterSlot::BOOTS,
			CharacterSlot::BRACERS,
			CharacterSlot::GLOVES,
			CharacterSlot::HAND_RIGHT,
			CharacterSlot::HAND_LEFT,
			CharacterSlot::CAPE,
			CharacterSlot::TABARD,
			CharacterSlot::BELT
	};

	if (traits.hasRobeBottom) {
		// if wearing a robe bottom, move boots before pants.
		auto boots = std::find(order.begin(), order.end(), CharacterSlot::BOOTS);
		auto pants = std::find(order.begin(), order.end(), CharacterSlot::PANTS);
		assert(boots != order.end());
		assert(pants != order.end());
		std::rotate(pants, boots, boots + 1);
	}

	assert(order.size() == (size_t)core::CharacterSlot::MAX);
	return order;
}