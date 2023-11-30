#pragma once

#include <QDialog>
#include "ui_Export3dDialog.h"
#include "core/modeling/Scene.h"
#include "core/database/GameDatabase.h"
#include "Formatting.h"

class Export3dDialog : public QDialog
{
	Q_OBJECT

public:
	Export3dDialog(core::GameDatabase* db, core::Scene* _scene, QWidget *parent = nullptr);
	~Export3dDialog();

private:
	Ui::Export3dDialogClass ui;

	core::Model* getTargetModel();
	void resetAnimations(core::Model* model);

	core::Scene* scene;
	core::GameDatabase* gameDB;

	AnimationOptions allAnimationOptions;
};
