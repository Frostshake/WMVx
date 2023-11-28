#include "stdafx.h"
#include "Export3dDialog.h"
#include "WMVxSettings.h"
#include "exporter/FbxExporter.h"
#include "core/utility/Logger.h"

Export3dDialog::Export3dDialog(core::Scene* _scene, QWidget* parent)
	: QDialog(parent), scene(_scene)
{
	ui.setupUi(this);

	assert(scene != nullptr);

	ui.comboBoxFormat->addItem("FBX");
	ui.lineEditOutput->setText(Settings::last3dDirectory() + "/model_export.fbx");

	connect(ui.pushButtonBrowse, &QPushButton::pressed, [&]() {

		QString filter = "All files(*.*)";
		auto outFile = QFileDialog::getSaveFileName(this, "Select File", ui.lineEditOutput->text(), filter, nullptr, QFileDialog::Option::DontConfirmOverwrite);
		if (outFile.isNull()) {
			return;
		}

		ui.lineEditOutput->setText(outFile);
	});

	connect(ui.pushButtonCancel, &QPushButton::pressed, [&]() {
		reject();
	});

	connect(ui.pushButtonSave, &QPushButton::pressed, [&]() {

		auto outFile = ui.lineEditOutput->text();

		if (outFile.isEmpty()) {
			QMessageBox::warning(this, "File name missing", "The output file must be specified.");
			return;
		}

		if (QFile::exists(outFile)) {
			auto result = QMessageBox::question(this, "File exists", "The output file already exists, overwrite?");

			if (result != QMessageBox::Yes) {
				return;
			}
		}

		try {
			exporter::FbxExporter exporter(outFile);
			//TODO handle multiple models / selection.
			if (!scene->models.empty()) {
				exporter.addModel(scene->models.front().get());
			}
			exporter.execute();
		}
		catch (std::exception e) {
			const QString what(e.what());
			core::Log::message("Exception thrown exporting FBX - " + what);
			QMessageBox::critical(this, "FBX Export Error", what)
				;
			return;
		}

		QFileInfo file_info(outFile);
		Settings::instance()->last3dDirectory = file_info.dir().absolutePath();
		Settings::instance()->save();

		accept();
	});

}

Export3dDialog::~Export3dDialog()
{}

void Export3dDialog::exportFBX(const QString& outputFileName) {


}