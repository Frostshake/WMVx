#include "stdafx.h"
#include "SceneControl.h"
#include "WMVxSettings.h"

using namespace core;

SceneControl::SceneControl(QWidget *parent)
	: QWidget(parent),
	WidgetUsesScene()
{
	ui.setupUi(this);

	ui.listWidgetModels->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	connect(ui.listWidgetModels, &QTreeWidget::customContextMenuRequested, [&](const QPoint& point) {

		if (ui.listWidgetModels->selectedItems().length() == 1) {
			QMenu menu(this);

			auto deleteAction = new QAction("Delete", &menu);
			connect(deleteAction, &QAction::triggered, [&]() {

				auto item = ui.listWidgetModels->selectedItems()[0];
				const auto item_id = item->data(Qt::UserRole).value<ModelId::value_t>();
				const auto item_index = ui.listWidgetModels->indexFromItem(item).row();

				assert(item_index >= 0);
				assert(item_index < scene->models.size());

				scene->removeModel(item_id);
				ui.listWidgetModels->takeItem(item_index);

				Log::message("Model Removed");
			});
			menu.addAction(deleteAction);

			menu.exec(ui.listWidgetModels->mapToGlobal(point));
		}
	});

	connect(ui.listWidgetModels, &QListWidget::itemActivated, [&](QListWidgetItem* item) {
		const auto item_id = item->data(Qt::UserRole).value<ModelId::value_t>();

		if (scene != nullptr) {
			auto model = std::find_if(scene->models.begin(), scene->models.end(), [item_id](const auto& m) {
				return m->id == item_id;
				});

			if (model != scene->models.end())
			{
				scene->setSelectedModel(model->get());
				return;
			}		
		}
		
		scene->setSelectedModel(nullptr);
	});
}

SceneControl::~SceneControl()
{}

void SceneControl::onSceneLoaded(core::Scene * new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::modelAdded, this, &SceneControl::onModelAdded);
	connect(scene, &Scene::modelSelectionChanged, this, &SceneControl::onModelSelectionChanged);
}


void SceneControl::onModelAdded(Model* model) {
	//TODO better way instead of reloading everything, also needs to remember which is selected (if applicable)
	ui.listWidgetModels->reset();
	ui.listWidgetModels->clear();

	if (scene != nullptr) {
		QListWidgetItem* match_widget = nullptr;
		for (const auto& item : scene->models) {
			QListWidgetItem* widget = new QListWidgetItem(ui.listWidgetModels);
			widget->setText(item->model->getFileInfo().toString());
			widget->setData(Qt::UserRole, QVariant::fromValue<ModelId::value_t>(item->id));
			ui.listWidgetModels->addItem(widget);
			if (item.get() == model) {
				match_widget = widget;
			}
		}


		if (model != nullptr && match_widget != nullptr) {
			if (Settings::get<bool>(config::app::auto_animate_new_models)) {
				const auto& anim_list = model->model->getModelAnimationSequenceAdaptors();
				const auto animation = std::find_if(anim_list.begin(), anim_list.end(),
					[](const core::ModelAnimationSequenceAdaptor* anim_adaptor) -> bool {
						return anim_adaptor->getId() == 0 && anim_adaptor->getVariationId() == 0;
					});

				if (animation != anim_list.end()) {
					model->animate = true;
					model->animator.setAnimation(*animation, animation - anim_list.begin());
				}
			}

			if (Settings::get<bool>(config::app::auto_focus_new_models)) {
				scene->setSelectedModel(model);
			}
		}
	}
}


void SceneControl::onModelSelectionChanged(core::Model* model) {
	if (model != nullptr) {
		for (auto i = 0; i < ui.listWidgetModels->count(); i++) {
			auto* item = ui.listWidgetModels->item(i);
			const auto item_id = item->data(Qt::UserRole).value<ModelId::value_t>();
			if (item_id == model->id) {
				ui.listWidgetModels->setCurrentItem(item, QItemSelectionModel::SelectionFlag::SelectCurrent);
			}
		}
	}
	else {
		ui.listWidgetModels->clearSelection();
	}
}