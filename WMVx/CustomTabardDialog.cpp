#include "stdafx.h"
#include "CustomTabardDialog.h"
#include <ranges>
#include <functional>
#include <algorithm>

using namespace core;

CustomTabardDialog::CustomTabardDialog(GameDatabase* db,
	GameFileSystem* fs,
	TabardCustomizationProvider* tcp,
	std::optional<core::CharacterItemWrapper> existing,
	std::optional<core::TabardCustomizationOptions> options,
	QWidget* parent)
	: QDialog(parent), existing_item(existing), existing_options(options)
{
	ui.setupUi(this);
	gameDB = db;
	gameFS = fs;
	tabardProvider = tcp;

	delayed_update = new Debounce(this, 100);

	connect(delayed_update, &Debounce::triggered, [&]() {
		chosenCustomisations.icon = ui.comboBoxIcon->currentIndex();
		chosenCustomisations.iconColor = ui.comboBoxIconColor->currentIndex();
		chosenCustomisations.border = ui.comboBoxBorder->currentIndex();
		chosenCustomisations.borderColor = ui.comboBoxBorderColor->currentIndex();
		chosenCustomisations.background = ui.comboBoxBackground->currentIndex();

		emit chosen(DialogChoiceMethod::PREVIEW, default_tabard_wrapper, chosenCustomisations);
	});

	auto possible_tabard_ids = std::views::keys(tabardProvider->getTieredCustomTabardItemIds());
	const auto* default_tabard = gameDB->itemsDB->findById(possible_tabard_ids.front());

	if (default_tabard == nullptr) {
		throw std::runtime_error("Unable to find custom tabard item.");
	}

	default_tabard_wrapper = CharacterItemWrapper::make(default_tabard, gameDB);

	customizationSizes = tabardProvider->getOptionsCount(); 

	for (uint32_t i = 0; i < customizationSizes.icon; i++) {
		ui.comboBoxIcon->addItem(QString::number(i));
	}

	for (uint32_t i = 0; i < customizationSizes.iconColor; i++) {
		ui.comboBoxIconColor->addItem(QString::number(i));
	}

	for (uint32_t i = 0; i < customizationSizes.border; i++) {
		ui.comboBoxBorder->addItem(QString::number(i));
	}

	for (uint32_t i = 0; i < customizationSizes.borderColor; i++) {
		ui.comboBoxBorderColor->addItem(QString::number(i));
	}

	for (uint32_t i = 0; i < customizationSizes.background; i++) {
		ui.comboBoxBackground->addItem(QString::number(i));
	}

	if (options.has_value()) {
		chosenCustomisations = options.value();
		ui.comboBoxIcon->setCurrentIndex(chosenCustomisations.icon);
		ui.comboBoxIconColor->setCurrentIndex(chosenCustomisations.iconColor);
		ui.comboBoxBorder->setCurrentIndex(chosenCustomisations.border);
		ui.comboBoxBorderColor->setCurrentIndex(chosenCustomisations.borderColor);
		ui.comboBoxBackground->setCurrentIndex(chosenCustomisations.background);
	}


	connect(ui.comboBoxIcon, &QComboBox::currentIndexChanged, delayed_update, &Debounce::absorb);
	connect(ui.comboBoxIconColor, &QComboBox::currentIndexChanged, delayed_update, &Debounce::absorb);
	connect(ui.comboBoxBorder, &QComboBox::currentIndexChanged, delayed_update, &Debounce::absorb);
	connect(ui.comboBoxBorderColor, &QComboBox::currentIndexChanged, delayed_update, &Debounce::absorb);
	connect(ui.comboBoxBackground, &QComboBox::currentIndexChanged, delayed_update, &Debounce::absorb);

	connect(ui.pushButtonCancel, &QPushButton::pressed, [&]() {
		emit chosen(DialogChoiceMethod::RESTORE, existing_item, existing_options);
		reject();
	});

	connect(ui.pushButtonApply, &QPushButton::pressed, [&]() {

		chosenCustomisations.icon = ui.comboBoxIcon->currentIndex();
		chosenCustomisations.iconColor = ui.comboBoxIconColor->currentIndex();
		chosenCustomisations.border = ui.comboBoxBorder->currentIndex();
		chosenCustomisations.borderColor = ui.comboBoxBorderColor->currentIndex();
		chosenCustomisations.background = ui.comboBoxBackground->currentIndex();

		emit chosen(DialogChoiceMethod::NEW, default_tabard_wrapper, chosenCustomisations);
		accept();
	});
	
}

CustomTabardDialog::~CustomTabardDialog()
{}

void CustomTabardDialog::closeEvent(QCloseEvent* e) {
	emit chosen(DialogChoiceMethod::RESTORE, existing_item, existing_options);
	QDialog::closeEvent(e);
}
