#include "stdafx.h"
#include "DevTools.h"
#include <algorithm>
#include <tuple>
#include <vector>
#include <map>

using namespace core;

DevTools::DevTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	model = nullptr;
	updatingGeosets = false;

	connect(ui.pushButtonRefreshGeosets, &QPushButton::pressed, this, &DevTools::updateModelData);


	connect(ui.treeWidgetGeosets, &QTreeWidget::itemChanged, [&](QTreeWidgetItem* item) {
		if (model != nullptr && !updatingGeosets) {
			updatingGeosets = true;
			const auto id = item->data(0, Qt::UserRole).toUInt();
			const bool visible = item->checkState(1) == Qt::Checked;

			model->forceGeosetVisibilityById(id, visible);
			for (const auto& merged : model->getMerged()) {
				merged->forceGeosetVisibilityById(id, visible);
			}


			for (auto index_item : checkboxes_by_geoset_index) {
				index_item.second->setCheckState(2, model->isGeosetIndexVisible(index_item.first) ? Qt::Checked : Qt::Unchecked);
			}

			const auto range = checkboxes_by_geoset_id.equal_range(id);
			for (auto itr = range.first; itr != range.second; ++itr) {
				if (itr->second != item) {
					itr->second->setCheckState(1, visible ? Qt::Checked : Qt::Unchecked);
				}
			}

			updatingGeosets = false;
		}
	});

	observeTimer = new QTimer(this);
	observeTimer->setInterval(500);
	
	connect(observeTimer, &QTimer::timeout, [&]() {
		updateAttachments();
	});

	ui.treeWidgetGeosets->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

DevTools::~DevTools()
{}

void DevTools::onSceneLoaded(core::Scene* new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::modelSelectionChanged, this, &DevTools::onModelChanged);

	if (model == nullptr) {
		onModelChanged(scene->selectedModel());
	}
}

void DevTools::onModelChanged(Model* target) {
	model = target;

	updateModelData();

	if (model == nullptr) {
		observeTimer->stop();
	}
	else {
		observeTimer->start();
	}
}


void DevTools::updateModelData()
{
	updateGeosets();
	updateAttachments();
	updateTextures();
}

void DevTools::updateGeosets() {

	updatingGeosets = true;
	//TODO ideally only clear the list if model is null, and smart update existing list.
	ui.treeWidgetGeosets->clear();

	checkboxes_by_geoset_id.clear();
	checkboxes_by_geoset_index.clear();

	if (model == nullptr) {
		return;
	}

	ui.treeWidgetGeosets->setDisabled(true);


	auto* top = createGeosetTreeNode(model, model->model.get(), "Root");
	top->setExpanded(true);
	ui.treeWidgetGeosets->addTopLevelItem(top);

	for (const auto& merged : model->getMerged()) {
		auto* merge_top = createGeosetTreeNode(model, model->model.get(), QString("Merged %1").arg(merged->getId()));
		ui.treeWidgetGeosets->addTopLevelItem(merge_top);
	}

	ui.treeWidgetGeosets->setDisabled(false);
	updatingGeosets = false;
}

void DevTools::updateAttachments()
{
	ui.treeAttachments->clear();

	if (model == nullptr) {
		return;
	}

	auto root = new QTreeWidgetItem(ui.treeAttachments);
	createAttachmentTreeItem(root, model, model->model.get());

	
	for (const auto* attach : model->getAttachments()) {
		auto item = new QTreeWidgetItem(ui.treeAttachments);
		createAttachmentTreeItem(item, nullptr, attach->getModel());

		for (const auto& enchant : attach->effects) {
			auto child = new QTreeWidgetItem(item);
			createAttachmentTreeItem(child, enchant.get(), enchant->model.get());
			item->addChild(child);
		}

		root->addChild(item);
	}

	ui.treeAttachments->addTopLevelItem(root);
}

void DevTools::updateTextures() {
	ui.listWidgetTextures->clear();

	if (model == nullptr) {
		return;
	}

	ui.listWidgetTextures->setDisabled(true);

	const auto insert_item = [&](const Texture* texture) {
		auto widget = new QListWidgetItem(ui.listWidgetTextures);
		widget->setText(texture->fileUri.toString());
		ui.listWidgetTextures->addItem(widget);
	};

	for (const auto& tex : model->textures) {
		insert_item(tex.second.get());
	}

	for (const auto& tex : model->replacableTextures) {
		insert_item(tex.second.get());
	}

	//TODO attachment textures.

	ui.listWidgetTextures->setDisabled(false);
}


inline QTreeWidgetItem* DevTools::createGeosetTreeNode(const core::Model* model, const core::RawModel* raw, QString name) {
	auto items = std::map<uint16_t, std::vector<uint32_t>>();

	uint32_t geoset_index = 0;
	for (const auto& geoset : raw->getGeosetAdaptors()) {
		const auto geoset_id = geoset->getId();
		
		if (!items.contains(geoset_id)) {
			items.insert({ geoset_id, { geoset_index } });
		}
		else {
			items.at(geoset_id).push_back(geoset_index);
		}
		
		geoset_index++;
	}


	for (auto& item : items) {
		std::sort(item.second.begin(), item.second.end());
	}



	auto* root = new QTreeWidgetItem(ui.treeWidgetGeosets);
	root->setText(0, name);

	std::map<core::CharacterGeosets, QTreeWidgetItem*> geoset_types;

	for (const auto& item : items) {
		auto type_num = core::CharacterGeosets(item.first / 100);

		QTreeWidgetItem* geoset_type = nullptr;

		if (!geoset_types.contains(type_num)) {
			QString str_name = "Unknown";
			if (Mapping::geosetNames.contains(type_num)) {
				str_name = Mapping::geosetNames.at(type_num);
			}

			geoset_type = new QTreeWidgetItem(root);
			geoset_type->setText(0, QString("%1xx | %2")
				.arg(type_num, 2, 10, QChar('0'))
				.arg(str_name));
			geoset_type->setExpanded(true);
;
			root->addChild(geoset_type);
			geoset_types.insert({ type_num, geoset_type });
		}
		else {
			geoset_type = geoset_types.at(type_num);
		}

		auto* geoset_item = new QTreeWidgetItem(geoset_type);
		geoset_item->setText(1, QString("%1").arg(item.first, 4, 10, QChar('0')));
		geoset_item->setData(0, Qt::UserRole, item.first);
		geoset_type->addChild(geoset_item);

		auto vis_count = 0;
		for (const auto& index : item.second) {
			const bool is_visible = model->isGeosetIndexVisible(index);
			auto* index_item = new QTreeWidgetItem(geoset_item);
			index_item->setText(2, QString::number(index));
			index_item->setCheckState(2, is_visible ? Qt::Checked : Qt::Unchecked);
			index_item->setDisabled(true);
			geoset_item->addChild(index_item);
			if (is_visible) {
				vis_count++;
			}

			checkboxes_by_geoset_index.insert({ index, index_item });
		}

		if (vis_count == 0) {
			geoset_item->setCheckState(1, Qt::Unchecked);
		}
		else if (vis_count == item.second.size()) {
			geoset_item->setCheckState(1, Qt::Checked);
		}
		else {
			geoset_item->setCheckState(1, Qt::PartiallyChecked);
		}

		checkboxes_by_geoset_id.insert({ item.first, geoset_item });

	}

	return root;
}

inline void DevTools::createAttachmentTreeItem(QTreeWidgetItem* item, const ModelTextureInfo* textures, const RawModel* model)
{
	item->setText(0, model->getFileInfo().toString());
	item->setText(1, QString::number(model->getTextureDefinitions().size()));
	
	{
		QStringList particleCounts;
		for (const auto& particle : model->getParticleAdaptors()) {
			particleCounts.push_back(QString::number(particle->getParticles().size()));
		}
		item->setText(2, particleCounts.join(" / "));
	}

	{
		QStringList segmentCounts;
		for (const auto& ribbon : model->getRibbonAdaptors()) {
			segmentCounts.push_back(QString::number(ribbon->getSegments().size()));
		}
		item->setText(3, segmentCounts.join(" / "));
	}

	item->setExpanded(true);
}
