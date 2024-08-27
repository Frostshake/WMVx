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
			const auto relation_index = item->data(1, Qt::UserRole).toInt();

			//managing the root model vs attachments has to work slightly differently.
			if (relation_index == -1) {
				// main model
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
			}
			else {
				// attachments
				const bool visible = item->checkState(2) == Qt::Checked;
				const auto geo_index = item->data(2, Qt::UserRole).toInt();


				std::visit([&](auto& attachData) {
	
					if constexpr (std::is_same_v<Attachment::AttachOwnedModel&, decltype(attachData)>) {
						attachData.forceGeosetVisibilityByIndex(geo_index, visible);
					}
					else if constexpr (std::is_same_v<Attachment::AttachMergedModel&, decltype(attachData)>) {
						attachData.model->forceGeosetVisibilityByIndex(geo_index, visible);
					}

				}, model->getAttachments().at(relation_index)->modelData);

				
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
	connect(scene, &Scene::modelSelectionChanged, this, &DevTools::onSceneSelectionChanged);

	if (model == nullptr) {
		onSceneSelectionChanged(scene->selected());
	}
}

void DevTools::onSceneSelectionChanged(const core::Scene::Selection& selection) {
	model = selection.root;

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
		if (merged->getType() == core::MergedModel::Type::CHAR_MODEL_ADDITION) {
			auto* merge_top = createGeosetTreeNode(merged, merged->model.get(), QString("Merged %1").arg(merged->getId()));
			ui.treeWidgetGeosets->addTopLevelItem(merge_top);
		}
	}

	int relation_index = 0;
	for (const auto& attach : model->getAttachments()) {

		std::visit([&](auto& attachData) {
			QString label = "Unknown Attachment";
			core::ModelGeosetInfo* geo = nullptr;
			core::M2Model* raw = nullptr;

			if constexpr (std::is_same_v<Attachment::AttachOwnedModel&, decltype(attachData)>) {
				label = QString("Attachment (Owned) %1 %2")
					.arg((int)attach->getSlot())
					.arg((int)attach->attachmentPosition);
				geo = &attachData;
				raw = attachData.model.get();
			}
			else if constexpr (std::is_same_v<Attachment::AttachMergedModel&, decltype(attachData)>) {
				label = QString("Attachment (Merged) %1 %2")
					.arg((int)attach->getSlot())
					.arg((int)attach->attachmentPosition);
				geo = attachData.model;
				raw = attachData.model->model.get();
			}

			auto* attach_top = createGeosetAttachmentTreeNode(geo, raw, label, relation_index++);
			ui.treeWidgetGeosets->addTopLevelItem(attach_top);

		}, attach->modelData);

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


QTreeWidgetItem* DevTools::createGeosetTreeNode(const core::ModelGeosetInfo* geoset_info, const core::M2Model* raw, QString name) {
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
		geoset_item->setData(1, Qt::UserRole, -1);
		geoset_type->addChild(geoset_item);

		auto vis_count = 0;
		for (const auto& index : item.second) {
			const bool is_visible = geoset_info->isGeosetIndexVisible(index);
			auto* index_item = new QTreeWidgetItem(geoset_item);
			index_item->setText(2, QString::number(index));
			index_item->setCheckState(2, is_visible ? Qt::Checked : Qt::Unchecked);
			index_item->setData(0, Qt::UserRole, item.first);
			index_item->setData(1, Qt::UserRole, -1);
			index_item->setData(2, Qt::UserRole, index);
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

QTreeWidgetItem* DevTools::createGeosetAttachmentTreeNode(const core::ModelGeosetInfo* geoset_info, const core::M2Model* raw, QString name, int relation_index) {
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
			geoset_type = new QTreeWidgetItem(root);
			geoset_type->setText(0, QString("%1xx")
				.arg(type_num, 2, 10, QChar('0')));
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
		geoset_type->addChild(geoset_item);

		auto vis_count = 0;
		for (const auto& index : item.second) {
			const bool is_visible = geoset_info->isGeosetIndexVisible(index);
			auto* index_item = new QTreeWidgetItem(geoset_item);
			index_item->setText(2, QString::number(index));
			index_item->setCheckState(2, is_visible ? Qt::Checked : Qt::Unchecked);
			index_item->setData(0, Qt::UserRole, item.first);
			index_item->setData(1, Qt::UserRole, relation_index);
			index_item->setData(2, Qt::UserRole, index);
			geoset_item->addChild(index_item);
		}


	}

	return root;
}

inline void DevTools::createAttachmentTreeItem(QTreeWidgetItem* item, const ModelTextureInfo* textures, const M2Model* model)
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

	item->setText(4, QString::number(model->getGeosetAdaptors().size()));

	item->setExpanded(true);
}
