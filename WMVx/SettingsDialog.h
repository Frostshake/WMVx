#pragma once

#include <QDialog>
#include "ui_SettingsDialog.h"
#include "WMVxSettings.h"

class SettingsDialog : public QDialog
{
	Q_OBJECT

public:
	SettingsDialog(QWidget *parent = nullptr);
	~SettingsDialog();

private:
	Ui::SettingsDialogClass ui;
};
