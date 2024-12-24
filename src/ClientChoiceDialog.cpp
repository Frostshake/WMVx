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
			.arg(QString::fromStdString(profile->versionString))
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
	env.product = ui.comboBoxProduct->currentData().toString();
	const auto found = WDBReader::Detector::all().detect(env.directory.toStdString());
	auto detected = found.end();

	for (auto it = found.begin(); it != found.end(); ++it) {
		if (QString::fromStdString(it->name) == env.product) {
			detected = it;
			break;
		}
	}

	if (detected != found.end()) {
		if (detected->locales.size() > 0) {
			env.locale = QString::fromStdString(detected->locales[0]);
		}
		env.version = detected->version;
	}
	else {
		env.version = { 0,0,0,0 };
		Log::message("Unable able to determine client environment.");
	}

	if (env.locale.isEmpty()) {
		env.locale = "enUS"; // assume enUS as a reasonable safe default.
		Log::message("Unable able to determine client locale.");
	}

	const auto& profile = *(availableProfiles[ui.comboBoxProfile->currentIndex()]);

	emit chosen(GameClientInfo(env, profile));

	accept();
}

void ClientChoiceDialog::detectVersion() {

	ui.labelDetectedVersion->setText("...");
	const auto found = WDBReader::Detector::all().detect(ui.lineEditFolderName->text().toStdString());

	ui.comboBoxProduct->clear();
	if (found.size() == 0) {
		ui.comboBoxProduct->addItem("Default", QString(""));
	}
	else {
		for (const auto& detected : found) {
			QString str = QString::fromStdString(detected.name);
			ui.comboBoxProduct->addItem(str.length() > 0 ? str : "Default", str);
		}
	}

	const auto detected = found.begin();
	
	if (found.size() > 0) {
		QStringList detected_str;
		for (const auto& install : found) {
			QString temp = "[";
			if (install.name.size() > 0) {
				temp += QString::fromStdString(install.name + " ");
			}
			temp += QString::fromStdString(install.version.toString());
			for (const auto& loc : install.locales) {
				temp += QString::fromStdString(" " + loc);
			}
			temp += "]";

			detected_str.push_back(std::move(temp));
		}

		ui.labelDetectedVersion->setText(detected_str.join(" "));

		if (detected != found.end()) {
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
				if (version.expansion >= (*it)->targetVersion.expansion) {
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

const std::array<const GameClientInfo::Profile*, 8> ClientChoiceDialog::availableProfiles = {
	&VanillaGameClientAdaptor::PROFILE,
	&TBCGameClientAdaptor::PROFILE,
	&WOTLKGameClientAdaptor::PROFILE,
	&CataGameClientAdaptor::PROFILE,
	&BFAGameClientAdaptor::PROFILE,
	&SLGameClientAdaptor::PROFILE,
	&DFGameClientAdaptor::PROFILE,
	&TWWGameClientAdaptor::PROFILE
};