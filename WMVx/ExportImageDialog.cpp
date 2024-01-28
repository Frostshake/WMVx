#include "stdafx.h"
#include "ExportImageDialog.h"
#include "WMVxSettings.h"
#include "WMVxVideoCapabilities.h"
#include "RenderWidget.h"

const QMap<QString,QString> ExportImageDialog::fileFormats = {
	{"BMP", "BMP (*.bmp)"},
	{"JPG", "JPEG (*.jpg *.jpeg)"},
	{"PNG", "PNG (*.png)"}
};

ExportImageDialog::ExportImageDialog(RenderWidget* widget, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	renderWidget = widget;
	assert(renderWidget != nullptr);

	ui.comboBoxFormat->addItems(fileFormats.keys());
	ui.lineEditOutput->setText(Settings::get(config::exporter::last_image_directory) + "/image_export.bmp");

	connect(ui.comboBoxFormat, &QComboBox::currentTextChanged, [&](QString text) {
		auto outFile = ui.lineEditOutput->text();
		QFileInfo file_info(outFile);
		auto suffix = file_info.completeSuffix();
		outFile = outFile.replace("."+suffix, "." + text.toLower());
		ui.lineEditOutput->setText(outFile);
	});

	connect(ui.pushButtonBrowse, &QPushButton::pressed, [&]() {

		QString filter = fileFormats[ui.comboBoxFormat->currentText()] + ";;All files (*.*)";
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

		auto image = screenshot();
		if (!image) {
			core::Log::message("Unable to create image.");
			return;
		}

		image->save(outFile);

		QFileInfo file_info(outFile);
		Settings::instance()->set(config::exporter::last_image_directory, file_info.dir().absolutePath());
		Settings::instance()->save();

		accept();
	});

}

ExportImageDialog::~ExportImageDialog()
{}

std::unique_ptr<QImage> ExportImageDialog::screenshot()
{
	std::unique_ptr<QImage> img = nullptr;

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	if (VideoCapabilities::support().frameBufferObject && false) {
		//TODO render to texture
	}
	else {
		// capturing from opengl is based on the whole window coordinates
		// crop image to only the render widget area
		auto size = renderWidget->size(); 
		auto geometry = renderWidget->geometry();
		auto bottomleft = renderWidget->mapTo(renderWidget->window(), geometry.bottomLeft());
		auto bottom_y = renderWidget->window()->height() - bottomleft.y() - 1;	//not sure why its out by 1?

		glReadBuffer(GL_BACK);
		img = std::unique_ptr<QImage>(new QImage(size.width(), size.height(), QImage::Format_RGB32));
		glReadPixels(bottomleft.x(), bottom_y, size.width(), size.height(), GL_BGRA_EXT, GL_UNSIGNED_BYTE, img->bits());
		*img = img->mirrored();
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 4);

	assert(img != nullptr);

	return std::move(img);
}
