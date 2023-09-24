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

public slots:
	void onModelChanged(core::Model* target);

private:

	Ui::ModelControlClass ui;

	void toggleActive();

	core::Model* model;
	bool isLoadingModel;
};
