#pragma once

#include <QDialog>
#include "ui_TextureTool.h"
#include "core/filesystem/GameFileSystem.h"

class TextureTool : public QDialog
{
	Q_OBJECT

public:
	TextureTool(QWidget *parent, core::GameFileSystem* fs);
	~TextureTool();

private:
	void tryOpenFile(const QString& name);
	void tryLoadImage(core::ArchiveFile* file);

	Ui::TextureToolClass ui;
	core::GameFileSystem* gameFS;

	QGraphicsScene* scene;
};
