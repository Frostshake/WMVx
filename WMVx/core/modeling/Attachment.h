#pragma once

#include <memory>
#include <vector>
#include "../game/GameConstants.h"
#include "RawModel.h"
#include "ModelSupport.h"
#include "Animator.h"
#include "../utility/Vector3.h"
#include "../game/GameClientAdaptor.h"

namespace core {
	class Attachment : public ModelTextureInfo, public ModelAnimationInfo
	{
	public:

		class Effect : public ModelTextureInfo, public ModelAnimationInfo {
		public:
			Effect(ModelFactory& factory) {
				model = factory();
			}
			Effect(Effect&&) = default;
			virtual ~Effect() {};

			void update(const Animator& animator, const AnimationTickArgs& tick) {

				model->calculateBones(animator.getAnimationIndex().value(), tick);
				updateAnimation();

				model->updateParticles(animator.getAnimationIndex().value(), tick);
				model->updateRibbons(animator.getAnimationIndex().value(), tick);
			}

			uint32_t itemVisualEffectId;
			std::unique_ptr<RawModel> model;
		};

		Attachment(ModelFactory& factory);
		Attachment(Attachment&&) = default;
		virtual ~Attachment() {}

		void update(const Animator& animator, const AnimationTickArgs& tick);

		AttachmentPosition attachmentPosition;
		CharacterSlot characterSlot;
		std::unique_ptr<RawModel> model;

		uint16_t bone;
		Vector3 position;

		std::vector<std::unique_ptr<Effect>> effects;
	};

}