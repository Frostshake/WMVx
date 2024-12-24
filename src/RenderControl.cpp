#include "stdafx.h"
#include "RenderControl.h"

using namespace core;

RenderControl::RenderControl(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	meta = nullptr;
	isLoadingModel = false;

	toggleActive();

	connect(ui.horizontalSliderAlpha, &QSlider::valueChanged, [&](int val) {
		if (meta != nullptr) {
			meta->renderOptions.opacity = float(val) / 100.0f;
		}
	});

	connect(ui.checkBoxWireFrame, &QCheckBox::stateChanged, [&]() {
		if (meta != nullptr) {
			meta->renderOptions.showWireFrame = ui.checkBoxWireFrame->isChecked();
		}
	});

	connect(ui.checkBoxBounds, &QCheckBox::stateChanged, [&]() {
		if (meta != nullptr) {
			meta->renderOptions.showBounds = ui.checkBoxBounds->isChecked();
		}
	});

	connect(ui.checkBoxBones, &QCheckBox::stateChanged, [&]() {
		if (meta != nullptr) {
			meta->renderOptions.showBones = ui.checkBoxBones->isChecked();
		}
	});

	connect(ui.checkBoxTexture, &QCheckBox::stateChanged, [&]() {
		if (meta != nullptr) {
			meta->renderOptions.showTexture = ui.checkBoxTexture->isChecked();
		}
	});

	connect(ui.checkBoxRender, &QCheckBox::stateChanged, [&]() {
		if (meta != nullptr) {
			meta->renderOptions.showRender = ui.checkBoxRender->isChecked();
		}
	});

	connect(ui.checkBoxParticles, &QCheckBox::stateChanged, [&]() {
		if (meta != nullptr) {
			meta->renderOptions.showParticles = ui.checkBoxParticles->isChecked();
		}
	});
}

RenderControl::~RenderControl()
{}


void RenderControl::onSceneLoaded(core::Scene* new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::modelSelectionChanged, this, &RenderControl::onSceneSelectionChanged);
}

void RenderControl::onSceneSelectionChanged(const core::Scene::Selection& selection) {
	meta = selection.component;
	toggleActive();
}

void RenderControl::toggleActive() {
	isLoadingModel = true;
	bool is_active = meta != nullptr;
	bool is_root = meta != nullptr && meta->getMetaType() == ComponentMeta::Type::ROOT;

	ui.labelAlpha->setDisabled(!is_active);
	ui.horizontalSliderAlpha->setDisabled(!is_active);
	ui.checkBoxWireFrame->setDisabled(!is_active || !is_root);
	ui.checkBoxBounds->setDisabled(!is_active || !is_root);
	ui.checkBoxBones->setDisabled(!is_active || !is_root);
	ui.checkBoxTexture->setDisabled(!is_active);
	ui.checkBoxRender->setDisabled(!is_active);
	ui.checkBoxParticles->setDisabled(!is_active);


	if (is_active) {
		ui.horizontalSliderAlpha->setValue(meta->renderOptions.opacity * 100);
		ui.checkBoxWireFrame->setChecked(meta->renderOptions.showWireFrame);
		ui.checkBoxBounds->setChecked(meta->renderOptions.showBounds);
		ui.checkBoxBones->setChecked(meta->renderOptions.showBones);
		ui.checkBoxTexture->setChecked(meta->renderOptions.showTexture);
		ui.checkBoxRender->setChecked(meta->renderOptions.showRender);
		ui.checkBoxParticles->setChecked(meta->renderOptions.showParticles);
	}
	else {
		ui.horizontalSliderAlpha->setValue(0);
		ui.checkBoxWireFrame->setChecked(false);
		ui.checkBoxBounds->setChecked(false);
		ui.checkBoxBones->setChecked(false);
		ui.checkBoxTexture->setChecked(false);
		ui.checkBoxRender->setChecked(false);
		ui.checkBoxParticles->setChecked(false);
	}

	isLoadingModel = false;
}