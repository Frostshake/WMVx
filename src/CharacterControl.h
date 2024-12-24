#pragma once

#include <QWidget>
#include "ui_CharacterControl.h"
#include "core/modeling/GenericModelAdaptors.h"
#include "core/modeling/Model.h"
#include "WidgetUsesScene.h"
#include "WidgetUsesGameClient.h"
#include "core/database/GameDatasetAdaptors.h"
#include "core/modeling/CharacterCustomization.h"
#include "core/modeling/AttachmentCustomization.h"


class CharacterControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	CharacterControl(QWidget *parent = nullptr);
	~CharacterControl();

	void onGameConfigLoaded(core::GameDatabase* db, core::GameFileSystem* fs, core::ModelSupport& ms) override;

	void onSceneLoaded(core::Scene* new_scene) override;

public slots:
	void onSceneSelectionChanged(const core::Scene::Selection& selection);

private:

	Ui::CharacterControlClass ui;

	void toggleActive();
	void openChoiceDialog(core::CharacterSlot slot);
	void updateEquipmentLabel(core::CharacterSlot slot);
	void randomiseComboBox(QComboBox* comboBox);
	void openEnchantDialog(core::CharacterSlot slot);

	void applyCustomizations();
	void updateModel();
	void updateEquipment();
	void updateItem(core::CharacterSlot slot, const core::CharacterItemWrapper& wrapper);

	core::GameFileUri searchSlotTexture(core::GameFileUri file, core::CharacterRegion region);

	void applyItemVisualToAttachment(core::Attachment* attachment, const core::ItemVisualRecordAdaptor* itemVisual, QString display_name = "");

	struct ButtonLabelPair {
		QPushButton* button;
		QLabel* label;
	};

	std::map<core::CharacterSlot, ButtonLabelPair> controlMap;

	core::Model* model;
	std::optional<core::CharacterRelationSearchContext> modelSearchContext;
	std::optional<core::CharacterRelationSearchContext> textureSearchContext;
	bool isLoadingModel;
	bool isRandomising;

	core::CharacterCustomizationOptions availableCustomizations;
	core::CharacterCustomizations chosenCustomisations; // chosen index	

	core::LegacyCharacterComponentTextureAdaptor legacyComponentTextureAdaptor;

	std::unique_ptr<core::TabardCustomizationProvider> tabardCustomizationProvider;
	std::unique_ptr<core::CharacterCustomizationProvider> characterCustomizationProvider;
	std::unique_ptr<core::AttachmentCustomizationProvider> attachmentCustomizationProvider;

	QComboBox* addCustomizationControl(const QString& name);
	QComboBox* getCustomizationControl(const QString& name);

	bool isCustomTabardEquiped() const;

	struct ModelTraits {
	public:
		ModelTraits(core::Model* model);
		bool hasRobeBottom;
	};

	std::vector<core::CharacterSlot> getSlotOrder(const ModelTraits& traits) const;
};

