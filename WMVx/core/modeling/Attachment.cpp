#include "../../stdafx.h"
#include "Attachment.h"

namespace core {

	Attachment::Attachment(ModelFactory& factory)
	{
		model = factory();
	}

	void Attachment::update(const Animator& animator, const AnimationTickArgs& tick)
	{
		model->calculateBones(animator.getAnimationIndex().value(), tick);
		updateAnimation();

		model->updateParticles(animator.getAnimationIndex().value(), tick);
		model->updateRibbons(animator.getAnimationIndex().value(), tick);

		for (auto& effect : effects) {
			effect->update(animator, tick);
		}
	}

}