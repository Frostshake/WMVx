#include "stdafx.h"
#include "AnimationControl.h"
#include "core/utility/Logger.h"
#include "core/game/GameClientAdaptor.h"

using namespace core;

AnimationControl::AnimationControl(QWidget* parent)
	: WidgetUsesGameClient(),
	QWidget(parent)
{
	ui.setupUi(this);
	model = nullptr;
	isLoadingModel = false;

	connect(ui.checkBoxAnimate, &QCheckBox::stateChanged, this, &AnimationControl::toggleAnimationsActive);

	connect(ui.comboBoxAnimations, &QComboBox::currentIndexChanged, this, &AnimationControl::selectAnimationFromIndex);

	connect(ui.horizontalSliderSpeed, &QSlider::valueChanged, [&]() {
		if (model != nullptr) {
			model->animator.setSpeed(ui.horizontalSliderSpeed->value() / 10.0f);
		}
	});

	connect(ui.pushButtonPlay, &QPushButton::pressed, [&]() {
		if (model != nullptr && model->animate) {
			model->animator.setPaused(false);

			if (!frameTimer->isActive()) {
				frameTimer->start();
			}
		}
		updateFrameSlider();
	});

	connect(ui.pushButtonStop, &QPushButton::pressed, [&]() {
		if (model != nullptr && model->animate) {
			model->animator.setPaused(true);

			if (frameTimer->isActive()) {
				frameTimer->stop();
			}
		}
		updateFrameSlider();
	});

	connect(ui.horizontalSliderFrame, &QSlider::valueChanged, [&]() {
		if (model != nullptr && model->animate && model->animator.isPaused()) {
			auto val = ui.horizontalSliderFrame->value();
			model->animator.setFrame(val);
		}
	});

	onModelChanged(nullptr);

	frameTimer = new QTimer(this);
	connect(frameTimer, &QTimer::timeout, this, [&]() {
		if (model != nullptr) {
			ui.labelFrameInfo->setText(
				QString("%1/%2").arg(model->animator.getCurrentFrame()).arg(model->animator.getTotalFrames() - 1)
			);
		}
		else {
			ui.labelFrameInfo->setText("");
		}
		});
	frameTimer->setInterval(100);
}

AnimationControl::~AnimationControl()
{}

void AnimationControl::toggleAnimationsActive()
{
	bool active = ui.checkBoxAnimate->isChecked();

	if (model != nullptr) {
		model->animate = active;
	}

	ui.comboBoxAnimations->setDisabled(!active);
	ui.labelSpeed->setDisabled(!active);
	ui.horizontalSliderSpeed->setDisabled(!active);
	ui.pushButtonPlay->setDisabled(!active);
	ui.pushButtonStop->setDisabled(!active);

	ui.labelFrame->setDisabled(!active);
	ui.labelFrameInfo->setVisible(active);

	updateFrameSlider();

	if (model != nullptr && active && ui.comboBoxAnimations->count() == 1) {
		//if we only have 1 animation, play
		selectAnimationFromIndex(0);
	}
}

void AnimationControl::updateFrameSlider() {

	bool enabled = model != nullptr && model->animate && model->animator.isPaused();

	if (enabled) {
		ui.horizontalSliderFrame->setDisabled(false);
		ui.horizontalSliderFrame->setMaximum(model->animator.getTotalFrames() - 1);
		ui.horizontalSliderFrame->setValue(model->animator.getCurrentFrame());
	}
	else {
		ui.horizontalSliderFrame->setDisabled(true);
		ui.horizontalSliderFrame->setMaximum(10);
		ui.horizontalSliderFrame->setValue(0);
	}
}

void AnimationControl::selectAnimationFromIndex(int index)
{
	if (model != nullptr && !isLoadingModel) {
		auto data = ui.comboBoxAnimations->itemData(index).toList();
		uint16_t animation_id = data[0].toInt();
		uint16_t variation_id = data[1].toInt();
		size_t animation_index = data[2].toInt();

		if (model != nullptr && gameDB != nullptr) {
			const auto& animation = model->model->getModelAnimationSequenceAdaptors().at(animation_index);
			model->animator.setAnimation(animation, animation_index);
		}
	}
}

void AnimationControl::onModelChanged(Model* target) {
	model = target;

	if (model != nullptr) {
		ui.checkBoxAnimate->setChecked(model->animate);
		ui.checkBoxAnimate->setDisabled(false);
	}
	else {
		ui.checkBoxAnimate->setChecked(false);
		ui.checkBoxAnimate->setDisabled(true);
	}

	isLoadingModel = true;

	ui.comboBoxAnimations->clear();

	if (model != nullptr && gameDB != nullptr) {

		auto animations = std::map<QString, QVariantList>();

		size_t animation_index = 0;
		QString select_match_name = "";
		for (const auto& animation : model->model->getModelAnimationSequenceAdaptors()) {
			auto const record = gameDB->animationDataDB->findById(animation->getId());

			if (record != nullptr) {
				QString name = record->getName() + QString(" [%1/%2]").arg(animation->getId()).arg(animation->getVariationId());
				
				//TODO THIS CHECK CURRENTLY NOT WORKING FOR VANILLA
				//assert(!animations.contains(name));

				animations[name] = {
					QVariant(animation->getId()),
					QVariant(animation->getVariationId()),
					QVariant(animation_index)
				};

				if (model->animator.getAnimationIndex().has_value()) {
					if (model->animator.getAnimationIndex().value() == animation_index) {
						select_match_name = name;
					}
				}
			}

			animation_index++;
		}

		for (const auto& item : animations) {
			ui.comboBoxAnimations->addItem(item.first,item.second);

			if (item.first == select_match_name && model->animator.getAnimationIndex().has_value()) {
				ui.comboBoxAnimations->setCurrentIndex(ui.comboBoxAnimations->count() - 1);
			}
		}

		ui.horizontalSliderSpeed->setValue(model->animator.getSpeed() * 10);


		if (model->animator.isPaused()) {
			if (frameTimer->isActive()) {
				frameTimer->stop();
			}
		}
		else {
			if (!frameTimer->isActive()) {
				frameTimer->start();
			}
		}

		Log::message(QString("Animations loaded: %1").arg(animations.size()));
	}

	toggleAnimationsActive();

	isLoadingModel = false;
}