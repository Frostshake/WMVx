#pragma once

#include <QDialog>
#include "ui_EquipmentChoiceDialog.h"
#include "core/database/GameDatabase.h"
#include "core/game/GameConstants.h"
#include "core/database/GameDataset.h"
#include "core/modeling/Model.h"
#include "Debounce.h"
#include "DialogChoiceMethod.h"

class EquipmentChoiceDialog : public QDialog
{
	Q_OBJECT

public:


	EquipmentChoiceDialog(core::GameDatabase* db, core::CharacterSlot slot, std::optional<core::CharacterItemWrapper> existing, QWidget *parent = nullptr);
	~EquipmentChoiceDialog();

signals:
	void chosen(DialogChoiceMethod method, core::CharacterSlot slot, std::optional<core::CharacterItemWrapper> wrapper);

protected:
	void closeEvent(QCloseEvent*) override;

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
	const core::CharacterSlot characterSlot;
	const std::optional<core::CharacterItemWrapper> existing_item;

	std::map<core::ItemQualityId, QCheckBox*> itemQualityFilters;
};
