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

	void onComponentAdded(core::ComponentMeta* meta);
	void onSelectionChanged(const core::Scene::Selection& selection);

private:

	void addSceneChild(QTreeWidgetItem* widget, core::ComponentMeta* meta, bool expand);

	Ui::SceneControlClass ui;

};
