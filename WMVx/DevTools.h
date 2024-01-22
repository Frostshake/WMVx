#pragma once

#include <QMainWindow>
#include "ui_DevTools.h"
#include "core/modeling/Model.h"
#include "WidgetUsesScene.h"


class DevTools : public QMainWindow, public WidgetUsesScene
{
	Q_OBJECT

public:
	DevTools(QWidget *parent = nullptr);
	~DevTools();

	void onSceneLoaded(core::Scene* new_scene) override;

public slots:
	void onModelChanged(core::Model* target);

private:
	Ui::DevToolsClass ui;

	void updateModelData();

	void updateGeosets();
	void updateAttachments();
	void updateTextures();

	inline void createTreeItem(QTreeWidgetItem* item,const core::ModelTextureInfo* textures, const core::RawModel* model);

	core::Model* model;

	bool updatingGeosets;

	QTimer* observeTimer;
};
