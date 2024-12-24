#pragma once

#include <QDialog>
#include "ui_EffectChoiceDialog.h"
#include "core/database/GameDatabase.h"
#include "core/game/GameConstants.h"
#include "core/database/GameDataset.h"


class EffectChoiceDialog : public QDialog
{
	Q_OBJECT

public:
	EffectChoiceDialog(core::GameDatabase* db, core::CharacterSlot slot, QWidget *parent = nullptr);
	~EffectChoiceDialog();

signals:
	void chosen(core::CharacterSlot slot, std::optional<uint32_t> spellEnchantId);

private:
	Ui::EffectChoiceDialogClass ui;

	core::GameDatabase* gameDB;
	core::CharacterSlot characterSlot;
};
