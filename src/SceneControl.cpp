#include "stdafx.h"
#include "SceneControl.h"
#include "WMVxSettings.h"

using namespace core;

SceneControl::SceneControl(QWidget* parent)
	: QWidget(parent),
	WidgetUsesScene()
{
	ui.setupUi(this);

	ui.treeWidgetModels->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	connect(ui.treeWidgetModels->itemDelegate(), &QAbstractItemDelegate::closeEditor, [&](QWidget* editor, QAbstractItemDelegate::EndEditHint hint) {
		auto* item = ui.treeWidgetModels->currentItem();
		if (item) {
			item->setFlags(item->flags() & ~Qt::ItemIsEditable);

			auto* meta = item->data(0, Qt::UserRole).value<ComponentMeta*>();
			assert(meta != nullptr);
			meta->setMetaName(item->text(0));
			item->setText(0, meta->getMetaLabel());
		}
	});

	connect(ui.treeWidgetModels, &QTreeWidget::customContextMenuRequested, [&](const QPoint& point) {

		if (ui.treeWidgetModels->selectedItems().length() == 1) {
			QMenu menu(this);
			auto* selected_item = ui.treeWidgetModels->selectedItems()[0];
			auto* meta = selected_item->data(0, Qt::UserRole).value<ComponentMeta*>();
			assert(meta != nullptr);

			auto renameAction = new QAction("Rename", &menu);
			connect(renameAction, &QAction::triggered, [&, selected_item]() {
				selected_item->setFlags(selected_item->flags() | Qt::ItemIsEditable);
				ui.treeWidgetModels->editItem(selected_item);
			});
			menu.addAction(renameAction);

			if (meta != nullptr && meta->getMetaType() == ComponentMeta::Type::ROOT) {
				auto deleteAction = new QAction("Delete", &menu);
				connect(deleteAction, &QAction::triggered, [&, selected_item, meta]() {
					const auto item_index = ui.treeWidgetModels->indexFromItem(selected_item).row();

					assert(item_index >= 0);
					assert(item_index < scene->models.size());

					scene->removeComponent(meta);
					ui.treeWidgetModels->takeTopLevelItem(item_index);

					Log::message("Model Removed");
				});
				menu.addAction(deleteAction);
			}

			menu.exec(ui.treeWidgetModels->mapToGlobal(point));
		}
		});

	connect(ui.treeWidgetModels, &QTreeWidget::itemActivated, [&](QTreeWidgetItem* item) {
		auto* meta = item->data(0, Qt::UserRole).value<ComponentMeta*>();
		Model* root_model = scene->findComponentRoot(meta);
		scene->setSelectedModel(root_model, meta);
	});
}

SceneControl::~SceneControl()
{}

void SceneControl::onSceneLoaded(core::Scene * new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::componentAdded, this, &SceneControl::onComponentAdded);
	connect(scene, &Scene::modelSelectionChanged, this, &SceneControl::onSelectionChanged);
	connect(scene, &Scene::sceneChanged, [&] { onComponentAdded(nullptr); });	//TODO tidy
}


void SceneControl::onComponentAdded(ComponentMeta* meta) {
	//TODO better way instead of reloading everything, also needs to remember which is selected (if applicable)
	ui.treeWidgetModels->reset();
	ui.treeWidgetModels->clear();

	if (scene != nullptr) {
		QTreeWidgetItem* match_widget = nullptr;
		Model* match_model = nullptr;

		for (const auto& item : scene->models) {
			const bool expand = item.get() == scene->selected().root;

			assert(item->getMetaType() == ComponentMeta::Type::ROOT);

			QTreeWidgetItem* widget = new QTreeWidgetItem(ui.treeWidgetModels);
			widget->setText(0, item->getMetaLabel());
			widget->setToolTip(0, item->getMetaGameFileInfo().toString());
			widget->setData(0, Qt::UserRole, QVariant::fromValue(static_cast<core::ComponentMeta*>(item.get())));
			widget->setText(1, "Root");
			widget->setExpanded(expand);

			ui.treeWidgetModels->addTopLevelItem(widget);
			
			for (auto* meta_child : item->getMetaChildren()) {
				addSceneChild(widget, meta_child, expand);
			}

			if (meta != nullptr) {
				if(meta == static_cast<ComponentMeta*>(item.get())) {
					match_widget = widget;
					match_model = item.get();
				}
			}
		}


		if (match_model != nullptr && match_widget != nullptr) {
			if (Settings::get<bool>(config::app::auto_animate_new_models)) {
				const auto& anim_list = match_model->model->getModelAnimationSequenceAdaptors();
				const auto animation = std::find_if(anim_list.begin(), anim_list.end(),
					[](const core::ModelAnimationSequenceAdaptor* anim_adaptor) -> bool {
						return anim_adaptor->getId() == 0 && anim_adaptor->getVariationId() == 0;
					});

				if (animation != anim_list.end()) {
					match_model->animate = true;
					match_model->animator.setAnimation(*animation, animation - anim_list.begin());
				}
			}

			if (Settings::get<bool>(config::app::auto_focus_new_models)) {
				scene->setSelectedModel(match_model, static_cast<ComponentMeta*>(match_model));
			}
		}
	}

	ui.treeWidgetModels->resizeColumnToContents(0);
}


void SceneControl::onSelectionChanged(const core::Scene::Selection& selection) {

	// Update the UI to reflect the chosen `selection`.

	std::function<bool(QTreeWidgetItem*)> check_children;

	check_children = [&](QTreeWidgetItem* twi) -> bool {
		for (auto i = 0; i < twi->childCount(); i++) {
			QTreeWidgetItem* child = twi->child(i);
			const auto* meta = child->data(0, Qt::UserRole).value<ComponentMeta*>();
			if (meta == selection.component) {
				ui.treeWidgetModels->setCurrentItem(child, QItemSelectionModel::SelectionFlag::SelectCurrent);
				return true;
			}

			if (check_children(child)) {
				return true;
			}
		}

		return false;
	};


	if (selection.component != nullptr) {
		for (auto i = 0; i < ui.treeWidgetModels->topLevelItemCount(); i++) {
			auto* item = ui.treeWidgetModels->topLevelItem(i);
			const auto* meta = item->data(0, Qt::UserRole).value<ComponentMeta*>();
			if (meta == selection.component) {
				ui.treeWidgetModels->setCurrentItem(item, QItemSelectionModel::SelectionFlag::SelectCurrent);
				return;
			}

			if (check_children(item)) {
				return;
			}
		}
	}

	if (selection.root != nullptr) {
		for (auto i = 0; i < ui.treeWidgetModels->topLevelItemCount(); i++) {
			auto* item = ui.treeWidgetModels->topLevelItem(i);
			const auto* meta = item->data(0, Qt::UserRole).value<ComponentMeta*>();
			if (meta == selection.root) {
				ui.treeWidgetModels->setCurrentItem(item, QItemSelectionModel::SelectionFlag::SelectCurrent);
				return;
			}
		}
	}
	
	ui.treeWidgetModels->clearSelection();
}

void SceneControl::addSceneChild(QTreeWidgetItem* widget, core::ComponentMeta* meta, bool expand)
{
	QTreeWidgetItem* child = new QTreeWidgetItem(widget);
	child->setText(0, meta->getMetaLabel());
	child->setToolTip(0, meta->getMetaGameFileInfo().toString());
	child->setData(0, Qt::UserRole, QVariant::fromValue(meta));
	child->setExpanded(expand);

#ifdef _DEBUG
	{
		auto* temp = child->data(0, Qt::UserRole).value<ComponentMeta*>();
		assert(temp == meta);
	}
#endif

	QString type_str = "?";

	
	switch (meta->getMetaType()) {
	case ComponentMeta::Type::ROOT:
		type_str = "Root";
		break;
	case ComponentMeta::Type::ATTACHMENT:
		type_str = QString("Attach (%1)").arg((int)static_cast<const Attachment*>(meta)->attachmentPosition);
		break;
	case ComponentMeta::Type::EFFECT:
		type_str = "Effect";
		break;
	case ComponentMeta::Type::MERGED:
		type_str = "Merged";
		const MergedModel* merged = static_cast<const MergedModel*>(meta);

		switch (merged->getType()) {
		case MergedModel::Type::CHAR_MODEL_ADDITION:
			type_str += " (Custom)";
			break;
		case MergedModel::Type::CHAR_ATTACHMENT_ADDITION:
			type_str += " (Attach)";
			break;
		default:
			type_str += " (?)";
			break;
		}

		break;
	}

	child->setText(1, type_str);

	for (auto* inner : meta->getMetaChildren()) {
		addSceneChild(child, inner, expand);
	}

	widget->addChild(child);
}
