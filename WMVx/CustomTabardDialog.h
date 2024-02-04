#pragma once

#include <QDialog>
#include "ui_CustomTabardDialog.h"
#include "core/database/GameDatabase.h"
#include "core/database/GameDataset.h"
#include "CharacterControl.h"
#include "core/modeling/TabardCustomization.h"
#include "DialogChoiceMethod.h"
#include "Debounce.h"

class CustomTabardDialog : public QDialog
{
	Q_OBJECT

public:
	CustomTabardDialog(core::GameDatabase* db, 
		core::GameFileSystem* fs, 
		core::TabardCustomizationProvider* tcp, 
		std::optional<core::CharacterItemWrapper> existing,
		std::optional<core::TabardCustomizationOptions> options,
		QWidget *parent = nullptr);
	~CustomTabardDialog();

signals:
	void chosen(DialogChoiceMethod method, std::optional<core::CharacterItemWrapper> wrapper,  std::optional<core::TabardCustomizationOptions> options);

protected:
	void closeEvent(QCloseEvent*) override;

private:
	Ui::CustomTabardDialogClass ui;
	
	core::TabardCustomizationOptions customizationSizes; //max sizes of options (e.g max_index + 1)
	core::TabardCustomizationOptions chosenCustomisations; // chosen index

	core::GameDatabase* gameDB;
	core::GameFileSystem* gameFS;
	core::TabardCustomizationProvider* tabardProvider;

	const std::optional<core::CharacterItemWrapper> existing_item;
	const std::optional<core::TabardCustomizationOptions> existing_options;

	Debounce* delayed_update;
	core::CharacterItemWrapper default_tabard_wrapper;

};
