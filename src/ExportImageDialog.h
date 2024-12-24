#pragma once

#include <QDialog>
#include "ui_ExportImageDialog.h"

class RenderWidget;

class ExportImageDialog : public QDialog
{
	Q_OBJECT

public:
	ExportImageDialog(RenderWidget* widget, QWidget *parent = nullptr);
	~ExportImageDialog();

	static const QMap<QString, QString> fileFormats;

private:

	std::unique_ptr<QImage> screenshot();

	Ui::ExportImageDialogClass ui;

	RenderWidget* renderWidget;
};
