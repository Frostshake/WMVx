#include "stdafx.h"
#include "ClientChoiceDialog.h"
#include "WMVxSettings.h"
#include "core/game/GameClientAdaptor.h"

using namespace core;

ClientChoiceDialog::ClientChoiceDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	auto knownFolder = Settings::get(config::client::game_folder);
	ui.lineEditFolderName->setText(knownFolder);
	ui.pushButtonLoad->setDisabled(knownFolder.length() == 0);

	for (const auto* profile : availableProfiles) {
		ui.comboBoxProfile->addItem(
			QString("%1 - %2")
			.arg(QString::fromStdString(profile->longName))
			.arg(profile->targetVersion)
		);
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
	GameClientInfo::Environment env;
	env.directory = ui.lineEditFolderName->text();
	
	const auto detected = GameClientInfo::detect(env.directory);
	if (detected.has_value()) {
		env.locale = detected->locale;
		env.version = detected->version;
	}
	else {
		env.locale = "enUS";
		env.version = { 0,0,0,0 };
		Log::message("Unable able to determine client environment.");
	}

	const auto& profile = *(availableProfiles[ui.comboBoxProfile->currentIndex()]);

	emit chosen(GameClientInfo(env, profile));

	accept();
}

void ClientChoiceDialog::detectVersion() {

	ui.labelDetectedVersion->setText("...");

	const auto detected = GameClientInfo::detect(ui.lineEditFolderName->text());

	if (detected.has_value()) {
		auto index = 0;
		const auto version = detected.value().version;
		ui.labelDetectedVersion->setText(version);

		for (const auto& profile : availableProfiles) {
			if (profile->targetVersion == version) {
				ui.comboBoxProfile->setCurrentIndex(index);
				return;
			}
			index++;
		}

		// if we cant get an exact match, try to use the closest instead.
		index = availableProfiles.size() - 1;
		for (auto it = availableProfiles.crbegin(); it != availableProfiles.crend(); ++it) {
			if (version.major >= (*it)->targetVersion.major) {
				break;
			}
			index--;
		}

		ui.comboBoxProfile->setCurrentIndex(index);		
	}
	else {
		ui.labelDetectedVersion->setText("Unable to detect client version.");
	}
}

const std::array<const GameClientInfo::Profile*, 4> ClientChoiceDialog::availableProfiles = {
	&VanillaGameClientAdaptor::PROFILE,
	&WOTLKGameClientAdaptor::PROFILE,
	&BFAGameClientAdaptor::PROFILE,
	&DFGameClientAdaptor::PROFILE
};