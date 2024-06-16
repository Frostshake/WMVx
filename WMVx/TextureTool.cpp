#include "stdafx.h"
#include "TextureTool.h"
#include "core/filesystem/GameFileUri.h"
#include "core/modeling/Texture.h"
#include "core/utility/ScopeGuard.h"

TextureTool::TextureTool(QWidget *parent, core::GameFileSystem* fs)
	: QDialog(parent), gameFS(fs)
{
	ui.setupUi(this);

	scene = new QGraphicsScene();
	ui.graphicsView->setScene(scene);

	connect(ui.pushButtonOpen, &QPushButton::pressed, [&]() {
		tryOpenFile(ui.lineEditName->text());
	});
}

TextureTool::~TextureTool()
{}

void TextureTool::tryOpenFile(const QString& name) {

	ui.labelWidth->setText("Width: ");
	ui.labelHeight->setText("Height: ");

	scene->clear();

	core::GameFileUri uri = 0u;
	
	bool is_number = false;
	uint32_t long_val = name.toUInt(&is_number);

	if (is_number) {
		uri = long_val;
	}
	else {
		uri = name;
	}

	std::unique_ptr<core::ArchiveFile> file = gameFS->openFile(uri);

	if (file == nullptr) {
		QMessageBox::warning(this, "File missing", "The specified file does not exist or cannot be openned.");
		return;
	}

	tryLoadImage(file.get());
}

void TextureTool::tryLoadImage(core::ArchiveFile* file) {

	core::BLPLoader loader(file);
	const auto& header = loader.getHeader();

	ui.labelWidth->setText("Width: " + QString::number(header.width));
	ui.labelHeight->setText("Height: " + QString::number(header.height));

	loader.loadFirst([&](int32_t mip, uint32_t w, uint32_t h, void* buffer) {
		QImage img((uchar*)buffer, w, h, QImage::Format::Format_RGBA8888);
		QGraphicsPixmapItem* item = new QGraphicsPixmapItem(QPixmap::fromImage(img));
		scene->addItem(item);
		ui.graphicsView->show();
	});

}