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
				auto item_index = ui.listWidgetModels->indexFromItem(item).row();

				assert(item_index >= 0);
				assert(item_index < scene->models.size());

				scene->removeModel(item_index);
				ui.listWidgetModels->takeItem(item_index);

				Log::message("Model Removed");
				emit selectedModalChanged(nullptr);
			});
			menu.addAction(deleteAction);

			menu.exec(ui.listWidgetModels->mapToGlobal(point));
		}
	});

	connect(ui.listWidgetModels, &QListWidget::itemActivated, [&](QListWidgetItem* item) {
		//TODO more reliable way other than using index.
		auto index = ui.listWidgetModels->indexFromItem(item);
		if (scene != nullptr && scene->models.size() > 0) {
			emit selectedModalChanged(scene->models.at(index.row()).get());
		}
		else {
			emit selectedModalChanged(nullptr);
		}
	});
}

SceneControl::~SceneControl()
{}


void SceneControl::onModelAdded() {
	//TODO better way instead of reloading everything, also needs to remember which is selected (if applicable)
	ui.listWidgetModels->reset();
	ui.listWidgetModels->clear();

	if (scene != nullptr) {
		QListWidgetItem* last = nullptr;
		for (const auto& item : scene->models) {
			QListWidgetItem* widget = new QListWidgetItem(ui.listWidgetModels);
			widget->setText(item->model->getFileInfo().toString());
			ui.listWidgetModels->addItem(widget);
			last = widget;
		}

		if (last != nullptr && !scene->models.empty()) {
			Model* model = scene->models.back().get();

			if (Settings::autoAnimateNewModels()) {
				const auto& anim_list = model->model->getModelAnimationSequenceAdaptors();
				const auto animation = std::find_if(anim_list.begin(),anim_list.end(),
					[](const core::ModelAnimationSequenceAdaptor* anim_adaptor) -> bool {
						return anim_adaptor->getId() == 0 && anim_adaptor->getVariationId() == 0;
					});

				if (animation != anim_list.end()) {
					model->animate = true;
					model->animator.setAnimation(*animation, animation - anim_list.begin());
				}
			}

			if (Settings::autoFocusNewModels()) {
				ui.listWidgetModels->setCurrentItem(last, QItemSelectionModel::SelectionFlag::SelectCurrent);

				emit selectedModalChanged(model);
			}
		}
	}
}