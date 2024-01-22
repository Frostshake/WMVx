#pragma once

#include <QWidget>
#include "ui_SceneControl.h"
#include "WidgetUsesScene.h"

class SceneControl : public QWidget, public WidgetUsesScene
{
	Q_OBJECT

public:
	SceneControl(QWidget *parent = nullptr);
	~SceneControl();

	void onSceneLoaded(core::Scene* new_scene) override;

public slots:

	void onModelAdded(core::Model* model);
	void onModelSelectionChanged(core::Model* model);

private:
	Ui::SceneControlClass ui;

};
