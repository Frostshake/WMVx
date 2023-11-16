#include "stdafx.h"
#include "DevTools.h"

using namespace core;

DevTools::DevTools(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	model = nullptr;
	updatingGeosets = false;

	connect(ui.pushButtonRefreshGeosets, &QPushButton::pressed, this, &DevTools::updateModelData);

	connect(ui.listWidgetGeosets, &QListWidget::itemChanged, [&](QListWidgetItem* item) {
		if (model != nullptr && !updatingGeosets) {
			auto index = item->data(Qt::UserRole).toInt();
			model->visibleGeosets[index] = item->checkState() == Qt::Checked;
		}
	});

	observeTimer = new QTimer(this);
	observeTimer->setInterval(500);
	
	connect(observeTimer, &QTimer::timeout, [&]() {
		updateAttachments();
	});
}

DevTools::~DevTools()
{}

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
	ui.listWidgetGeosets->clear();

	if (model == nullptr) {
		return;
	}

	ui.listWidgetGeosets->setDisabled(true);

	auto items = std::map<uint32_t, std::pair<QString, size_t>>();

	//sort items by geoset id.
	auto geoset_index = 0;
	for (auto geoset : model->model->getGeosetAdaptors()) {
		items[geoset->getId()] = { QString::number(geoset->getId()), geoset_index++ };
	}

	for (const auto& item : items) {
		auto widget = new QListWidgetItem(ui.listWidgetGeosets);
		widget->setText(item.second.first);
		widget->setData(Qt::UserRole, QVariant(item.second.second));
		widget->setCheckState(model->visibleGeosets[item.second.second] ? Qt::Checked : Qt::Unchecked);
		ui.listWidgetGeosets->addItem(widget);
	}

	ui.listWidgetGeosets->setDisabled(false);
	updatingGeosets = false;
}

void DevTools::updateAttachments()
{
	ui.treeAttachments->clear();

	if (model == nullptr) {
		return;
	}

	auto root = new QTreeWidgetItem(ui.treeAttachments);
	createTreeItem(root, model, model->model.get());

	for (const auto* attach : model->getAttachments()) {
		auto item = new QTreeWidgetItem(ui.treeAttachments);
		createTreeItem(item, attach, attach->model.get());

		for (const auto& enchant : attach->effects) {
			auto child = new QTreeWidgetItem(item);
			createTreeItem(child, enchant.get(), enchant->model.get());
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

	ui.listWidgetTextures->setDisabled(false);
}

inline void DevTools::createTreeItem(QTreeWidgetItem* item, const ModelTextureInfo* textures, const RawModel* model)
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
