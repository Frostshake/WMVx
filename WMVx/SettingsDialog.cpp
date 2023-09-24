#include "stdafx.h"
#include "SettingsDialog.h"
#include "WMVx.h"
#include "WMVxSettings.h"
#include "WMVxVideoCapabilities.h"

SettingsDialog::SettingsDialog(QWidget* parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	ui.lineEditGameFolder->setText(Settings::gameFolder());

	assert(VideoCapabilities::isLoaded());

	auto modes = VideoCapabilities::availableModes();

	std:sort(modes.begin(), modes.end(), [](const WMVxVideoCapabilities::DisplayMode& lhs, const WMVxVideoCapabilities::DisplayMode& rhs) -> bool {
		
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

	//TODO preselect active item

	//TODO connect saving active item


	connect(ui.pushButtonChangeFolder, &QPushButton::pressed, [&]() {
		auto directory = QFileDialog::getExistingDirectory(this, "Choose game directory", ui.lineEditGameFolder->text());
		ui.lineEditGameFolder->setText(directory);
	});

	connect(ui.pushButtonApply, &QPushButton::pressed, [&]() {
		Settings::instance()->gameFolder = ui.lineEditGameFolder->text();
		Settings::instance()->save();

		accept();
	});
}

SettingsDialog::~SettingsDialog()
{
}
