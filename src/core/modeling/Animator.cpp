#include "../../stdafx.h"
#include "Animator.h"

namespace core {

	Animator::Animator()
	{
		paused = false;
		totalFrames = 0;
		speed = 1.0f;
	}

	void Animator::setAnimation(const ModelAnimationSequenceAdaptor* animation, size_t animation_index)
	{
		totalFrames = animation->getDuration();
		animationId = animation->getId();
		animationIndex = animation_index;
	}

	const AnimationTickArgs& Animator::tick(uint32_t delta_time_msecs)
	{
		if (!paused) {
			lastTick.deltaTime = delta_time_msecs * speed;
			lastTick.absoluteTime += lastTick.deltaTime;
			lastTick.currentFrame += lastTick.deltaTime;
			
			const auto frameMax = totalFrames - 1;
			if (lastTick.currentFrame > frameMax) {
				lastTick.currentFrame -= frameMax;
			}
		}
		else {
			lastTick.deltaTime = 0;
		}

		return lastTick;
	}

	float Animator::getSpeed() const
	{
		return speed;
	}

	void Animator::setSpeed(float speed)
	{
		this->speed = speed;
	}

	void Animator::setFrame(uint32_t frame)
	{
		lastTick.currentFrame = frame;
	}

	const AnimationTickArgs& Animator::getLastTick() const
	{
		return lastTick;
	}

	uint32_t Animator::getCurrentFrame() const {
		return lastTick.currentFrame;
	}

	uint32_t Animator::getTotalFrames() const {
		return totalFrames;
	}

	std::optional<uint16_t> Animator::getAnimationId() const {
		return animationId;
	}

	std::optional<size_t> Animator::getAnimationIndex() const {
		return animationIndex;
	}

	void Animator::setPaused(bool p)
	{
		paused = p;
	}

	bool Animator::isPaused() const
	{
		return paused;
	}
}