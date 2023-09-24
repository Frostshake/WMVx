#pragma once

#include <QDialog>
#include "ui_Export3dDialog.h"

class Export3dDialog : public QDialog
{
	Q_OBJECT

public:
	Export3dDialog(QWidget *parent = nullptr);
	~Export3dDialog();

private:
	Ui::Export3dDialogClass ui;
};
