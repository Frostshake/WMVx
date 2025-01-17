#include "stdafx.h"
#include "ModelControl.h"
#include "core/game/GameClientAdaptor.h"
#include <QJsonDocument>

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


	connect(ui.horizontalSliderScale, &QSlider::valueChanged, [&](int val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->modelOptions.scale.x = float(val) / 10.f;
			model->modelOptions.scale.y = float(val) / 10.f;
			model->modelOptions.scale.z = float(val) / 10.f;

			ui.doubleSpinBoxScaleX->setValue(model->modelOptions.scale.x);
			ui.doubleSpinBoxScaleY->setValue(model->modelOptions.scale.y);
			ui.doubleSpinBoxScaleZ->setValue(model->modelOptions.scale.z);
		}
	});

	connect(ui.doubleSpinBoxScaleX, &QDoubleSpinBox::valueChanged, [&](double val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->modelOptions.scale.x = (float)val;
			ui.horizontalSliderScale->setValue(model->modelOptions.scale.max() * 10.f);
		}
	});

	connect(ui.doubleSpinBoxScaleY, &QDoubleSpinBox::valueChanged, [&](double val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->modelOptions.scale.y = (float)val;
			ui.horizontalSliderScale->setValue(model->modelOptions.scale.max() * 10.f);
		}
	});

	connect(ui.doubleSpinBoxScaleZ, &QDoubleSpinBox::valueChanged, [&](double val) {
		std::unique_lock<std::mutex> lock(scale_mutex, std::try_to_lock);
		if (model != nullptr && lock.owns_lock()) {
			model->modelOptions.scale.z = (float)val;
			ui.horizontalSliderScale->setValue(model->modelOptions.scale.max() * 10.f);
		}
	});

	connect(ui.doubleSpinBoxPositionX, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->modelOptions.position.x = (float)val;
		}
	});

	connect(ui.doubleSpinBoxPositionY, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->modelOptions.position.y = (float)val;
		}
	});

	connect(ui.doubleSpinBoxPositionZ, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->modelOptions.position.z = (float)val;
		}
	});

	connect(ui.doubleSpinBoxYaw, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->modelOptions.rotation.z = (float)val;
		}
	});

	connect(ui.doubleSpinBoxPitch, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->modelOptions.rotation.y = (float)val;
		}
	});

	connect(ui.doubleSpinBoxRoll, &QDoubleSpinBox::valueChanged, [&](double val) {
		if (model != nullptr) {
			model->modelOptions.rotation.x = (float)val;
		}
	});

	connect(ui.comboBoxSkinsPreset, &QComboBox::currentIndexChanged, [&](int index) {
		if (model != nullptr && gameFS != nullptr && scene != nullptr && ui.comboBoxSkinsPreset->currentText().length() > 0) {

			for (auto& texture_group : model->textureSet.groups) {
				auto group_id_str = QString::number(texture_group.id);
				if (ui.comboBoxSkinsPreset->currentText() == group_id_str) {

					Log::message("Loading model skin: " + group_id_str);

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

	connect(ui.pushButtonAttachInfo, &QPushButton::pressed, [&] {
		if (model != nullptr) {
			QJsonArray arr;
			const auto& lookups = model->model->getAttachmentLookups();
			for (const auto& lookup : lookups) {
				if (lookup != static_cast<decltype(lookup)>(-1)) {
					const auto& def = model->model->getAttachmentDefintionAdaptors().at(lookup);
					QJsonObject att_obj{
						{"AttachmentPosition", Mapping::attachmentPositionNames.at((AttachmentPosition)lookup)},
						{"Id", (int64_t)def->getId()},
						{"Bone", (int64_t)def->getBone()}
					};

					arr.push_back(std::move(att_obj));
				}
			}

			QJsonDocument doc(arr);
			QClipboard* clipboard = QGuiApplication::clipboard();
			clipboard->setText(doc.toJson());
			QToolTip::showText(QCursor::pos(), "Copied", ui.pushButtonAttachInfo);
		}

	});

	connect(ui.pushButtonBoneInfo, &QPushButton::pressed, [&] {
		if (model != nullptr) {

		}
	});
}

ModelControl::~ModelControl()
{}

void ModelControl::onSceneLoaded(core::Scene* new_scene)
{
	WidgetUsesScene::onSceneLoaded(new_scene);
	connect(scene, &Scene::modelSelectionChanged, this, &ModelControl::onSceneSelectionChanged);
}


void ModelControl::onSceneSelectionChanged(const core::Scene::Selection& selection) {
	if (selection.component && selection.component->getMetaType() == ComponentMeta::Type::ROOT) {
		model = selection.root;
	}
	else {
		model = nullptr;
	}
	toggleActive();
}


void ModelControl::toggleActive() {

	isLoadingModel = true;
	bool is_active = model != nullptr;

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

	ui.pushButtonAttachInfo->setDisabled(!is_active);
	ui.pushButtonBoneInfo->setDisabled(true /*!is_active*/);

	if (is_active) {
		ui.horizontalSliderScale->setValue(model->modelOptions.scale.max() * 10.f);
		ui.horizontalSliderScale->setValue(model->modelOptions.scale.max() * 10.f);
		ui.doubleSpinBoxScaleX->setValue(model->modelOptions.scale.x);
		ui.doubleSpinBoxScaleY->setValue(model->modelOptions.scale.y);
		ui.doubleSpinBoxScaleZ->setValue(model->modelOptions.scale.z);

		ui.doubleSpinBoxPositionX->setValue(model->modelOptions.position.x);
		ui.doubleSpinBoxPositionY->setValue(model->modelOptions.position.y);
		ui.doubleSpinBoxPositionZ->setValue(model->modelOptions.position.z);

		ui.doubleSpinBoxYaw->setValue(model->modelOptions.rotation.z);
		ui.doubleSpinBoxPitch->setValue(model->modelOptions.rotation.y);
		ui.doubleSpinBoxRoll->setValue(model->modelOptions.rotation.x);

		std::vector<uint32_t> skin_options;
		skin_options.reserve(model->textureSet.groups.size());

		for (auto& texture_group : model->textureSet.groups) {
			skin_options.push_back(texture_group.id);
		}

		std::sort(skin_options.begin(), skin_options.end());

		for (auto skin_id : skin_options) {
			ui.comboBoxSkinsPreset->addItem(QString::number(skin_id));
		}

		ui.labelM2Magic->setText(QString::fromLatin1(
			reinterpret_cast<const char*>(model->model->getMagic().data()),
			model->model->getMagic().size()
		));
		ui.labelM2Version->setText(QString::number(model->model->getHeader().version));
		QStringList chunks_str;
		for (const auto& chunk : model->model->getChunks()) {
			chunks_str += QString::fromLatin1(
				reinterpret_cast<const char*>(chunk.first.data()),
				chunk.first.size()
			);
		}
		ui.labelM2Chunks->setText(chunks_str.join('\n'));

	}
	else {
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

		ui.labelM2Magic->setText("");
		ui.labelM2Version->setText("");
		ui.labelM2Chunks->setText("");
	}

	isLoadingModel = false;
}