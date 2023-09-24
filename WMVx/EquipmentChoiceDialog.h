#pragma once

#include <QDialog>
#include "ui_EquipmentChoiceDialog.h"
#include "core/database/GameDatabase.h"
#include "core/game/GameConstants.h"
#include "core/database/GameDataset.h"
#include "Debounce.h"

class EquipmentChoiceDialog : public QDialog
{
	Q_OBJECT

public:
	EquipmentChoiceDialog(core::GameDatabase* db, core::CharacterSlot slot, QWidget *parent = nullptr);
	~EquipmentChoiceDialog();

signals:
	void chosen(core::CharacterSlot slot, const core::ItemRecordAdaptor* record);

private:

	inline QVariant itemToVariant(const core::ItemRecordAdaptor* record) {
		return QVariantList({ record->getId() , (uint32_t)record->getItemQuality() });
	}

	inline std::pair<uint32_t, core::ItemQualityId> variantToItem(const QVariant& variant) {
		QVariantList item_data = variant.toList();

		return {
			item_data[0].toUInt(),
			static_cast<core::ItemQualityId>(item_data[1].toUInt())
		};
	}

	Ui::EquipmentChoiceDialogClass ui;

	Debounce* delayedSearch;

	core::GameDatabase* gameDB;
	core::CharacterSlot characterSlot;

	std::map<core::ItemQualityId, QCheckBox*> itemQualityFilters;
};
