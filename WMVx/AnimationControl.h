#pragma once

#include <QWidget>
#include "ui_AnimationControl.h"
#include "core/modeling/Model.h"
#include "WidgetUsesGameClient.h"

class AnimationControl : public QWidget, public WidgetUsesGameClient
{
	Q_OBJECT

public:
	AnimationControl(QWidget *parent = nullptr);
	~AnimationControl();

public slots:
	void onModelChanged(core::Model* target);
	void toggleAnimationsActive();
	void updateFrameSlider();

private:
	Ui::AnimationControlClass ui;

	void selectAnimationFromIndex(int index);

	core::Model* model;
	bool isLoadingModel;

	QTimer* frameTimer;
};
