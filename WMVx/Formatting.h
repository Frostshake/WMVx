#pragma once

#include "core/modeling/Model.h"

#include <map>
#include <tuple>
#include <QString>
#include <QVariant>

struct AnimationOption {
public:
	uint16_t id;
	uint16_t variationId;
	size_t index;
};

Q_DECLARE_METATYPE(AnimationOption)

using AnimationOptions = std::map<QString, AnimationOption>;

class Formatting {
public:
	static AnimationOptions animationOptions(core::GameDatabase* gameDB, core::Model* model) {
		AnimationOptions values;

		if (!gameDB->animationDataDB) {
			return values;
		}

		size_t animation_index = 0;
		for (const auto& animation : model->model->getModelAnimationSequenceAdaptors()) {
			auto const record = gameDB->animationDataDB->findById(animation->getId());

			if (record != nullptr) {
				QString name = QString("%1 [%2/%3]")
					.arg(record->getName())
					.arg(animation->getId())
					.arg(animation->getVariationId());

				//TODO THIS CHECK CURRENTLY NOT WORKING FOR VANILLA
				//assert(!animations.contains(name));

				values.insert({
					name,
					{
						animation->getId(),
						animation->getVariationId(),
						animation_index
					}
					});

			}

			animation_index++;
		}


		return values;
	}

};
