#pragma once

#include <QDialog>
#include "ui_Export3dDialog.h"
#include "core/modeling/Scene.h"

class Export3dDialog : public QDialog
{
	Q_OBJECT

public:
	Export3dDialog(core::Scene* _scene, QWidget *parent = nullptr);
	~Export3dDialog();

private:
	Ui::Export3dDialogClass ui;

	void exportFBX(const QString& outputFileName);

	core::Scene* scene;
};
