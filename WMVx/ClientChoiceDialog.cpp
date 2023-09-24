#include "stdafx.h"
#include "ClientChoiceDialog.h"
#include "WMVxSettings.h"

using namespace core;

ClientChoiceDialog::ClientChoiceDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	auto knownFolder = Settings::gameFolder();
	ui.lineEditFolderName->setText(knownFolder);
	ui.pushButtonLoad->setDisabled(knownFolder.length() == 0);

	for (auto& version : availableVersions) {
		ui.comboBoxVersion->addItem(version);
	}

	if (knownFolder.length() > 0) {
		detectVersion();
	}

	connect(ui.pushButtonBrowse, &QPushButton::pressed, [&]() {
		auto directory = QFileDialog::getExistingDirectory(this, "Choose game directory", ui.lineEditFolderName->text());
		if (directory.isNull()) {
			return;
		}

		ui.lineEditFolderName->setText(directory);
		ui.pushButtonLoad->setDisabled(directory.length() == 0);
		detectVersion();
	});

	connect(ui.pushButtonLoad, &QPushButton::pressed, this, &ClientChoiceDialog::load);
}

ClientChoiceDialog::~ClientChoiceDialog()
{}

void ClientChoiceDialog::load()
{
	auto info = GameClientInfo();
	info.directory = ui.lineEditFolderName->text();
	info.locale = "enUS";
	info.version = availableVersions[ui.comboBoxVersion->currentIndex()];

	emit chosen(info);

	accept();
}

void ClientChoiceDialog::detectVersion() {
	const auto detected = GameClientInfo::detect(ui.lineEditFolderName->text());

	if (detected.has_value()) {
		auto index = 0;
		for (const auto ver : availableVersions) {
			if (ver == detected.value().version) {
				ui.comboBoxVersion->setCurrentIndex(index);
				break;
			}
			index++;
		}
	}
}

const GameClientVersion ClientChoiceDialog::availableVersions[3] = {
	GameClientVersion(1, 12, 1, 5875),
	GameClientVersion(3, 3, 5, 12340),
	GameClientVersion(8, 3, 7, 35435)
};