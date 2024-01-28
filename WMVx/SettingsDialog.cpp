#include "stdafx.h"
#include "SettingsDialog.h"
#include "WMVx.h"
#include "WMVxSettings.h"
#include "WMVxVideoCapabilities.h"
#include "ArcBallCamera.h"
#include "BasicCamera.h"

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEditGameFolder->setText(Settings::get(config::client::game_folder));

	assert(VideoCapabilities::isLoaded());

	auto modes = VideoCapabilities::availableModes();

	std::sort(modes.begin(), modes.end(), [](const WMVxVideoCapabilities::DisplayMode& lhs, const WMVxVideoCapabilities::DisplayMode& rhs) -> bool {
		
		if (lhs.colorBits != rhs.colorBits) {
			return lhs.colorBits > rhs.colorBits;
		}

		if (lhs.depthBits != rhs.depthBits) {
			return lhs.depthBits > rhs.depthBits;
		}

		if (lhs.alphaBits != rhs.alphaBits) {
			return lhs.alphaBits > rhs.alphaBits;
		}

		if (lhs.antiAliasingSamples != rhs.antiAliasingSamples) {
			return lhs.antiAliasingSamples > rhs.antiAliasingSamples;
		}
		
		if (lhs.stencilBits != rhs.stencilBits) {
			return lhs.stencilBits > rhs.stencilBits;
		}
				
		if (lhs.doubleBuffer != rhs.doubleBuffer) {
			return lhs.doubleBuffer;
		}	
				
		return lhs.acceleration.compare(rhs.acceleration) > 0;			
	});

	for (const auto& mode : modes) {
		QString mode_string = QString("Color:%1 Depth:%2 Alpha:%3 AA:%4 Stencil:%5")
			.arg(mode.colorBits)
			.arg(mode.depthBits)
			.arg(mode.alphaBits)
			.arg(mode.antiAliasingSamples)
			.arg(mode.stencilBits);

		if (mode.doubleBuffer) {
			mode_string += " DoubleBuffer";
		}

		mode_string += " " + mode.acceleration;

		ui.comboBoxDisplayMode->addItem(mode_string);
	}

	ui.comboBoxDisplayMode->setDisabled(true); //TODO remove - disable until implemented.

	//TODO preselect active item

	//TODO connect saving active item

	const auto cam_type = Settings::get(config::rendering::camera_type);
	ui.radioButtonArcball->setChecked(cam_type == ArcBallCamera::identifier);
	ui.radioButtonBasic->setChecked(cam_type == BasicCamera::identifier);
	ui.checkBoxHideCursor->setChecked(Settings::get<bool>(config::rendering::camera_hide_mouse));

	connect(ui.pushButtonChangeFolder, &QPushButton::pressed, [&]() {
		auto directory = QFileDialog::getExistingDirectory(this, "Choose game directory", ui.lineEditGameFolder->text());
		ui.lineEditGameFolder->setText(directory);
	});

	connect(ui.pushButtonApply, &QPushButton::pressed, [&]() {
		Settings::instance()->set(config::client::game_folder, ui.lineEditGameFolder->text());

		if (ui.radioButtonArcball->isChecked()) {
			Settings::instance()->set(config::rendering::camera_type, ArcBallCamera::identifier);
		}
		else if (ui.radioButtonBasic->isChecked()) {
			Settings::instance()->set(config::rendering::camera_type, BasicCamera::identifier);
		}

		Settings::instance()->set(config::rendering::camera_hide_mouse, ui.checkBoxHideCursor->isChecked());

		Settings::instance()->save();

		accept();
	});
}

SettingsDialog::~SettingsDialog()
{
}
