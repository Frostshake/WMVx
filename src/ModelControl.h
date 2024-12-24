#pragma once

#include <QWidget>
#include "ui_ModelControl.h"
#include "core/modeling/Model.h"
#include "core/utility/Logger.h"
#include "WidgetUsesScene.h"
#include "WidgetUsesGameClient.h"

class ModelControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	ModelControl(QWidget *parent = nullptr);
	~ModelControl();

	void onSceneLoaded(core::Scene* new_scene) override;

public slots:
	void onSceneSelectionChanged(const core::Scene::Selection& selection);

private:

	Ui::ModelControlClass ui;

	void toggleActive();

	core::Model* model;
	bool isLoadingModel;

	std::mutex scale_mutex;
};
