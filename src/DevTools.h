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
	void onSceneSelectionChanged(const core::Scene::Selection& selection);

private:
	Ui::DevToolsClass ui;

	void updateModelData();

	void updateGeosets();
	void updateAttachments();
	void updateTextures();

	QTreeWidgetItem* createGeosetTreeNode(const core::ModelGeosetInfo* geoset_info, const core::M2Model* raw, QString name);
	QTreeWidgetItem* createGeosetAttachmentTreeNode(const core::ModelGeosetInfo* geoset_info, const core::M2Model* raw, QString name, int relation_index);
	inline void createAttachmentTreeItem(QTreeWidgetItem* item,const core::ModelTextureInfo* textures, const core::M2Model* model);

	void geosetOverrideChange(QTreeWidgetItem* item, Qt::CheckState state);

	core::Model* model;

	bool updatingGeosets;

	QTimer* observeTimer;


	std::multimap<uint16_t, QTreeWidgetItem*> checkboxes_by_geoset_id;
	std::multimap<uint32_t, QTreeWidgetItem*> checkboxes_by_geoset_index;
};
