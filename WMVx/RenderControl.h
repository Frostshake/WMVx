#pragma once

#include <QWidget>
#include "ui_RenderControl.h"
#include "core/modeling/Model.h"
#include "WidgetUsesScene.h"
#include "WidgetUsesGameClient.h"

class RenderControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	RenderControl(QWidget *parent = nullptr);
	~RenderControl();

	void onSceneLoaded(core::Scene* new_scene) override;

public slots:
	void onSceneSelectionChanged(const core::Scene::Selection& selection);

private:
	Ui::RenderControlClass ui;

	void toggleActive();

	core::ComponentMeta* meta;
	bool isLoadingModel;
};
