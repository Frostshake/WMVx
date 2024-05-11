#pragma once

#include <QWidget>
#include "ui_AnimationControl.h"
#include "core/modeling/Model.h"
#include "WidgetUsesGameClient.h"
#include "WidgetUsesScene.h"

class AnimationControl : public QWidget, public WidgetUsesScene, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	AnimationControl(QWidget *parent = nullptr);
	~AnimationControl();

	void onSceneLoaded(core::Scene* new_scene) override;

public slots:
	void onSceneSelectionChanged(const core::Scene::Selection& selection);
	void toggleAnimationsActive();
	void updateFrameSlider();

private:
	Ui::AnimationControlClass ui;

	void selectAnimationFromIndex(int index);

	core::Model* model;
	bool isLoadingModel;

	QTimer* frameTimer;
};
