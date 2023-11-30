#pragma once

#include <QDialog>
#include "ui_CustomTabardDialog.h"
#include "core/database/GameDatabase.h"
#include "core/database/GameDataset.h"
#include "CharacterControl.h"
#include "core/modeling/TabardCustomization.h"

class CustomTabardDialog : public QDialog
{
	Q_OBJECT

public:
	CustomTabardDialog(core::GameDatabase* db, core::GameFileSystem* fs, core::TabardCustomizationProvider* tcp, QWidget *parent = nullptr);
	~CustomTabardDialog();

signals:
	void chosen(core::TabardCustomizationOptions tabard);

private:
	Ui::CustomTabardDialogClass ui;
	
	core::TabardCustomizationOptions customizationSizes; //max sizes of options (e.g max_index + 1)
	core::TabardCustomizationOptions chosenCustomisations; // chosen index

	core::GameDatabase* gameDB;
	core::GameFileSystem* gameFS;
	core::TabardCustomizationProvider* tabardProvider;
};
