#include "stdafx.h"
#include "ModelControl.h"
#include "core/game/GameClientAdaptor.h"

using namespace core;

ModelControl::ModelControl(QWidget* parent)
	: WidgetUsesScene(),
	WidgetUsesGameClient(),
	QWidget(parent)
{
	ui.setupUi(this);
	model = nullptr;
	isLoadingModel = false;

	ui.doubleSpinBoxPitch->setWrapping(true);
	ui.doubleSpinBoxRoll->setWrapping(true);
	ui.doubleSpinBoxYaw->setWrapping(true);

	toggleActive();

	connect(ui.horizontalSliderAlpha, &QSlider::valueChanged, [&](int val) {
		if (model != nullptr) {
			model->renderOptions.opacity = float(val) / 100.0f;
		}
	});

	connect(ui.checkBoxWireFrame, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr) {
			model->renderOptions.showWireFrame = ui.checkBoxWireFrame->isChecked();
		}
	});

	connect(ui.checkBoxBounds, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr) {
			model->renderOptions.showBounds = ui.checkBoxBounds->isChecked();
		}
	});

	connect(ui.checkBoxBones, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr) {
			model->renderOptions.showBones = ui.checkBoxBones->isChecked();
		}
	});

	connect(ui.checkBoxTexture, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr) {
			model->renderOptions.showTexture = ui.checkBoxTexture->isChecked();
		}
	});

	connect(ui.checkBoxRender, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr) {
			model->renderOptions.showRender = ui.checkBoxRender->isChecked();
		}
	});

	connect(ui.checkBoxParticles, &QCheckBox::stateChanged, [&]() {
		if (model != nullptr) {
			model->renderOptions.showParticles = ui.checkBoxParticles->isChecked();
		}
	});

	connect(ui.horizontalSliderScale, &QSlider::valueChanged, [&](int val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->renderOptions.scale.x = float(val) / 10.f;
			model->renderOptions.scale.y = float(val) / 10.f;
			model->renderOptions.scale.z = float(val) / 10.f;

			ui.doubleSpinBoxScaleX->setValue(model->renderOptions.scale.x);
			ui.doubleSpinBoxScaleY->setValue(model->renderOptions.scale.y);
			ui.doubleSpinBoxScaleZ->setValue(model->renderOptions.scale.z);
		}
	});

	connect(ui.doubleSpinBoxScaleX, &QDoubleSpinBox::valueChanged, [&](double val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->renderOptions.scale.x = (float)val;
			ui.horizontalSliderScale->setValue(model->renderOptions.scale.max() * 10.f);
		}
	});

	connect(ui.doubleSpinBoxScaleY, &QDoubleSpinBox::valueChanged, [&](double val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->renderOptions.scale.y = (float)val;
			ui.horizontalSliderScale->setValue(model->renderOptions.scale.max() * 10.f);
		}
	});

	connect(ui.doubleSpinBoxScaleZ, &QDoubleSpinBox::valueChanged, [&](double val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->renderOptions.scale.z = (float)val;
			ui.horizontalSliderScale->setValue(model->renderOptions.scale.max() * 10.f);
		}
	});

	connect(ui.doubleSpinBoxPositionX, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->renderOptions.position.x = (float)val;
		}
	});

	connect(ui.doubleSpinBoxPositionY, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->renderOptions.position.y = (float)val;
		}
	});

	connect(ui.doubleSpinBoxPositionZ, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->renderOptions.position.z = (float)val;
		}
	});

	connect(ui.doubleSpinBoxYaw, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->renderOptions.rotation.z = (float)val;
		}
	});

	connect(ui.doubleSpinBoxPitch, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->renderOptions.rotation.y = (float)val;
		}
	});

	connect(ui.doubleSpinBoxRoll, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->renderOptions.rotation.x = (float)val;
		}
	});

	connect(ui.comboBoxSkinsPreset, &QComboBox::currentIndexChanged, [&](int index) {
		if (model != nullptr && gameFS != nullptr && scene != nullptr && ui.comboBoxSkinsPreset->currentText().length() > 0) {

			for (auto& texture_group : model->textureSet.groups) {
				if (ui.comboBoxSkinsPreset->currentText() == texture_group.texture[0].toString()) {

					Log::message("Loading model skin: " + texture_group.texture[0].toString());

					for (auto i = 0; i < texture_group.textureCount; i++) {
						auto base = texture_group.base + i;
						const auto& texture_spec = texture_group.texture[i];

						GameFileUri blpFile = "";
						if (texture_spec.isPath()) {
							QString container = model->model->getFileInfo().toString();
							container = container.left(container.lastIndexOf("\\"));
							blpFile = container + "\\" + texture_spec.getPath() + ".blp";
						}
						else {
							blpFile = texture_spec;
						}

						auto texture = scene->textureManager.add(blpFile, gameFS);
						if (texture != nullptr) {
							model->replacableTextures[(TextureType)base] = texture;
						}
					}

					break;
				}
			}
		}
	});
}

ModelControl::~ModelControl()
{}

void ModelControl::onSceneLoaded(core::Scene* new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::modelSelectionChanged, this, &ModelControl::onModelChanged);
}


void ModelControl::onModelChanged(Model* target) {
	model = target;
	toggleActive();
}


void ModelControl::toggleActive() {

	isLoadingModel = true;
	bool is_active = model != nullptr;

	ui.horizontalSliderAlpha->setDisabled(!is_active);
	ui.checkBoxWireFrame->setDisabled(!is_active);
	ui.checkBoxBounds->setDisabled(!is_active);
	ui.checkBoxBones->setDisabled(!is_active);
	ui.checkBoxTexture->setDisabled(!is_active);
	ui.checkBoxRender->setDisabled(!is_active);
	ui.checkBoxParticles->setDisabled(!is_active);

	ui.horizontalSliderScale->setDisabled(!is_active);
	ui.doubleSpinBoxScaleX->setDisabled(!is_active);
	ui.doubleSpinBoxScaleY->setDisabled(!is_active);
	ui.doubleSpinBoxScaleZ->setDisabled(!is_active);

	ui.doubleSpinBoxPositionX->setDisabled(!is_active);
	ui.doubleSpinBoxPositionY->setDisabled(!is_active);
	ui.doubleSpinBoxPositionZ->setDisabled(!is_active);

	ui.doubleSpinBoxYaw->setDisabled(!is_active);
	ui.doubleSpinBoxPitch->setDisabled(!is_active);
	ui.doubleSpinBoxRoll->setDisabled(!is_active);

	ui.comboBoxSkinsPreset->setDisabled(!is_active);
	ui.comboBoxSkinsPreset->clear();

	if (is_active) {
		ui.horizontalSliderAlpha->setValue(model->renderOptions.opacity * 100);
		ui.checkBoxWireFrame->setChecked(model->renderOptions.showWireFrame);
		ui.checkBoxBounds->setChecked(model->renderOptions.showBounds);
		ui.checkBoxBones->setChecked(model->renderOptions.showBones);
		ui.checkBoxTexture->setChecked(model->renderOptions.showTexture);
		ui.checkBoxRender->setChecked(model->renderOptions.showRender);
		ui.checkBoxParticles->setChecked(model->renderOptions.showParticles);

		ui.horizontalSliderScale->setValue(model->renderOptions.scale.max() * 10.f);
		ui.horizontalSliderScale->setValue(model->renderOptions.scale.max() * 10.f);
		ui.doubleSpinBoxScaleX->setValue(model->renderOptions.scale.x);
		ui.doubleSpinBoxScaleY->setValue(model->renderOptions.scale.y);
		ui.doubleSpinBoxScaleZ->setValue(model->renderOptions.scale.z);

		ui.doubleSpinBoxPositionX->setValue(model->renderOptions.position.x);
		ui.doubleSpinBoxPositionY->setValue(model->renderOptions.position.y);
		ui.doubleSpinBoxPositionZ->setValue(model->renderOptions.position.z);

		ui.doubleSpinBoxYaw->setValue(model->renderOptions.rotation.z);
		ui.doubleSpinBoxPitch->setValue(model->renderOptions.rotation.y);
		ui.doubleSpinBoxRoll->setValue(model->renderOptions.rotation.x);

		for (auto& texture_group : model->textureSet.groups) {
			ui.comboBoxSkinsPreset->addItem(texture_group.texture[0].toString());
		}
	}
	else {
		ui.horizontalSliderAlpha->setValue(0);
		ui.checkBoxWireFrame->setChecked(false);
		ui.checkBoxBounds->setChecked(false);
		ui.checkBoxBones->setChecked(false);
		ui.checkBoxTexture->setChecked(false);
		ui.checkBoxRender->setChecked(false);
		ui.checkBoxParticles->setChecked(false);

		ui.horizontalSliderScale->setValue(1);
		ui.doubleSpinBoxScaleX->setValue(1);
		ui.doubleSpinBoxScaleY->setValue(1);
		ui.doubleSpinBoxScaleZ->setValue(1);

		ui.doubleSpinBoxPositionX->setValue(0);
		ui.doubleSpinBoxPositionY->setValue(0);
		ui.doubleSpinBoxPositionZ->setValue(0);

		ui.doubleSpinBoxYaw->setValue(0);
		ui.doubleSpinBoxPitch->setValue(0);
		ui.doubleSpinBoxRoll->setValue(0);
	}

	isLoadingModel = false;
}