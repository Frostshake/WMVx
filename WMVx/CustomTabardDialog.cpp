#include "stdafx.h"
#include "CustomTabardDialog.h"

using namespace core;

CustomTabardDialog::CustomTabardDialog(GameDatabase* db, GameFileSystem* fs, TabardCustomizationProvider* tcp, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	gameDB = db;
	gameFS = fs;
	tabardProvider = tcp;

	customizationSizes = tabardProvider->getOptionsCount(); 

	for (auto i = 0; i < customizationSizes.icon; i++) {
		ui.comboBoxIcon->addItem(QString::number(i));
	}

	for (auto i = 0; i < customizationSizes.iconColor; i++) {
		ui.comboBoxIconColor->addItem(QString::number(i));
	}

	for (auto i = 0; i < customizationSizes.border; i++) {
		ui.comboBoxBorder->addItem(QString::number(i));
	}

	for (auto i = 0; i < customizationSizes.borderColor; i++) {
		ui.comboBoxBorderColor->addItem(QString::number(i));
	}

	for (auto i = 0; i < customizationSizes.background; i++) {
		ui.comboBoxBackground->addItem(QString::number(i));
	}

	connect(ui.pushButtonCancel, &QPushButton::pressed, this, &QDialog::reject);
	connect(ui.pushButtonApply, &QPushButton::pressed, [&]() {

		chosenCustomisations.icon = ui.comboBoxIcon->currentIndex();
		chosenCustomisations.iconColor = ui.comboBoxIconColor->currentIndex();
		chosenCustomisations.border = ui.comboBoxBorder->currentIndex();
		chosenCustomisations.borderColor = ui.comboBoxBorderColor->currentIndex();
		chosenCustomisations.background = ui.comboBoxBackground->currentIndex();

		emit chosen(chosenCustomisations);

		accept();
	});
	
}

CustomTabardDialog::~CustomTabardDialog()
{}

