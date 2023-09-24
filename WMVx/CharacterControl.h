#pragma once

#include <QWidget>
#include "ui_CharacterControl.h"
#include "core/modeling/GenericModelAdaptors.h"
#include "core/modeling/Model.h"
#include "WidgetUsesScene.h"
#include "WidgetUsesGameClient.h"
#include "core/database/GameDatasetAdaptors.h"


class CharacterControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	CharacterControl(QWidget *parent = nullptr);
	~CharacterControl();

	void onGameConfigLoaded(core::GameDatabase* db, core::GameFileSystem* fs, core::ModelSupport& ms) override;

	struct CharacterDetails {
		uint32_t raceId = 0;
		core::Gender gender = core::Gender::MALE;
	};

	struct CharacterCustomizationOptions {
		uint32_t skin = 0;
		uint32_t face = 0;
		uint32_t hairColour = 0;
		uint32_t hairStyle = 0;
		uint32_t facialStyle = 0;
		uint32_t facialColour = 0;

		void reset(uint32_t val = 0) {
			skin = val;
			face = val;
			hairColour = val;
			hairStyle = val;
			facialStyle = val;
			facialColour = val;
		}
	};

	

public slots:
	void onModelChanged(core::Model* target);

private:

	Ui::CharacterControlClass ui;

	void toggleActive();
	void openChoiceDialog(core::CharacterSlot slot);
	void updateEquipmentLabel(core::CharacterSlot slot);
	void randomiseComboBox(QComboBox* comboBox);
	void openEnchantDialog(core::CharacterSlot slot);

	void refreshCustomisationRecords();
	void setGeosetVisibility(core::CharacterGeosets geoset, uint32_t id);
	void updateModel();
	void updateEquipment();
	void updateItem(core::CharacterSlot slot, const core::ItemRecordAdaptor* item);

	core::GameFileUri searchSlotTexture(core::GameFileUri file, core::CharacterRegion region);

	void applyItemVisualToAttachment(core::Attachment* attachment, const core::ItemVisualRecordAdaptor* itemVisual);

	std::vector < core::AttachmentPosition > getAttachmentPositions(core::CharacterSlot slot, const core::ItemRecordAdaptor* item);

	struct ButtonLabelPair {
		QPushButton* button;
		QLabel* label;
	};

	std::map<core::CharacterSlot, ButtonLabelPair> controlMap;

	core::Model* model;
	std::optional<CharacterDetails> characterDetails;
	bool isLoadingModel;
	bool isRandomising;

	CharacterCustomizationOptions customizationSizes; //max sizes of options (e.g max_index + 1)
	CharacterCustomizationOptions chosenCustomisations; // chosen index	

	core::LegacyCharacterComponentTextureAdaptor legacyComponentTextureAdaptor;

	std::unique_ptr<core::TabardCustomizationProvider> tabardCustomizationProvider;

};

