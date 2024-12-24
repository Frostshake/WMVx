#include "stdafx.h"
#include "Export3dDialog.h"
#include "WMVxSettings.h"
#include "exporter/FbxExporter.h"
#include "core/utility/Logger.h"

Export3dDialog::Export3dDialog(core::GameDatabase* db, core::Scene* _scene, QWidget* parent)
	: QDialog(parent), gameDB(db), scene(_scene)
{
	ui.setupUi(this);

	assert(scene != nullptr);

	ui.comboBoxFormat->addItem("FBX");
	ui.lineEditOutput->setText(Settings::get(config::exporter::last_3d_directory) + "/model_export.fbx");

	{
		auto* target = getTargetModel();
		if (target != nullptr) {
			resetAnimations(target);
		}
	}

	connect(ui.pushButtonBrowse, &QPushButton::pressed, [&]() {

		QString filter = "All files(*.*)";
		auto outFile = QFileDialog::getSaveFileName(this, "Select File", ui.lineEditOutput->text(), filter, nullptr, QFileDialog::Option::DontConfirmOverwrite);
		if (outFile.isNull()) {
			return;
		}

		ui.lineEditOutput->setText(outFile);
	});

	auto bulk_check = [&](bool val) {
		auto checkboxes = ui.scrollAreaAnimations->findChildren<QCheckBox*>();
		for(auto* checkbox : checkboxes) {
			checkbox->setChecked(val);
		}
	};

	connect(ui.pushButtonSelect, &QPushButton::pressed, std::bind(bulk_check, true));

	connect(ui.pushButtonUnselected, &QPushButton::pressed, std::bind(bulk_check, false));

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
			auto* target = getTargetModel();
			if (target != nullptr) {
				AnimationOptions selected;
				auto checkboxes = ui.scrollAreaAnimations->findChildren<QCheckBox*>();
				for (auto* checkbox : checkboxes) {
					if (checkbox->isChecked()) {
						const auto key = checkbox->text();
						selected[key] = allAnimationOptions[key];
					}
				}

				exporter.addModel(target, selected);
			}
			exporter.execute();
		}
		catch (std::exception e) {
			const QString what(e.what());
			core::Log::message("Exception thrown exporting FBX - " + what);
			QMessageBox::critical(this, "FBX Export Error", what);
			return;
		}

		QFileInfo file_info(outFile);
		Settings::instance()->set(config::exporter::last_3d_directory, file_info.dir().absolutePath());
		Settings::instance()->save();

		accept();
	});

}

Export3dDialog::~Export3dDialog()
{}

core::Model* Export3dDialog::getTargetModel()
{
	//TODO ability to choose model.
	if (scene->models.size() > 0) {
		return scene->models.front().get();
	}

	return nullptr;
}

void Export3dDialog::resetAnimations(core::Model* model)
{
	QWidget* container = new QWidget();
	QVBoxLayout* layout = new QVBoxLayout(container);
	ui.scrollAreaAnimations->setWidget(container);

	if (model != nullptr) {
		allAnimationOptions = Formatting::animationOptions(gameDB, model);
	
		for (const auto& option : allAnimationOptions) {
			QCheckBox* checkbox = new QCheckBox();
			checkbox->setText(option.first);
			layout->addWidget(checkbox);
		}
	}


}

