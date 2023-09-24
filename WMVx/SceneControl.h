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

public slots:

	void onModelAdded();

signals:
	void selectedModalChanged(core::Model* model);

private:
	Ui::SceneControlClass ui;

};
