#include "stdafx.h"
#include "ClientChoiceDialog.h"
#include "WMVxSettings.h"
#include "core/game/GameClientAdaptor.h"

using namespace core;

std::optional<WDBReader::Detection::ClientInfo> findDefaultWowInstall(const  WDBReader::Detection::Detector::result_t& detected) {
	auto wow_match = std::find_if(detected.begin(), detected.end(), [](const auto& result) {
		return result.name == "wow" || result.name == "";
	});

	if (wow_match != detected.end()) {
		return *wow_match;
	}

	return std::nullopt;
}

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
			.arg(QString::fromStdString(profile->targetVersion))
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
	const auto found = WDBReader::Detection::Detector::all().detect(env.directory.toStdString());
	const auto detected = findDefaultWowInstall(found);
	if (detected.has_value()) {
		env.locale = QString::fromStdString(detected->locale);
		env.version = detected->version;
	}
	else {
		env.locale = "enUS"; // assume enUS as a reasonable safe default.
		env.version = { 0,0,0,0 };
		Log::message("Unable able to determine client environment.");
	}

	const auto& profile = *(availableProfiles[ui.comboBoxProfile->currentIndex()]);

	emit chosen(GameClientInfo(env, profile));

	accept();
}

void ClientChoiceDialog::detectVersion() {

	ui.labelDetectedVersion->setText("...");
	const auto found = WDBReader::Detection::Detector::all().detect(ui.lineEditFolderName->text().toStdString());
	const auto detected = findDefaultWowInstall(found);
	
	if (found.size() > 0) {
		QStringList detected_str;
		for (const auto& install : found) {
			QString temp = "[";
			if (install.name.size() > 0) {
				temp += QString::fromStdString(install.name + " ");
			}
			temp += QString::fromStdString(install.version.toString());
			if (install.locale.size() > 0) {
				temp += QString::fromStdString(" " + install.locale);
			}
			temp += "]";

			detected_str.push_back(std::move(temp));
		}

		ui.labelDetectedVersion->setText(detected_str.join(" "));

		if (detected.has_value()) {
			auto index = 0;
			const auto version = detected->version;

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