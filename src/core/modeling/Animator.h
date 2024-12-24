#pragma once
#include <cstdint>
#include <optional>
#include "ModelAdaptors.h"
#include "Animation.h"

namespace core {
	class Animator
	{
	public:
		Animator();
		Animator(Animator&&) = default;

		void setAnimation(const ModelAnimationSequenceAdaptor*, size_t animation_index);
		const AnimationTickArgs& tick(uint32_t delta_time_msecs);

		float getSpeed() const;
		void setSpeed(float speed);

		void setFrame(uint32_t frame);

		const AnimationTickArgs& getLastTick() const;

		uint32_t getCurrentFrame() const;
		uint32_t getTotalFrames() const;

		std::optional<uint16_t> getAnimationId() const;
		std::optional<size_t> getAnimationIndex() const;

		void setPaused(bool p);
		bool isPaused() const;

	protected:
		bool paused;

		uint32_t totalFrames;
		std::optional< uint16_t> animationId;
		std::optional<size_t> animationIndex;

		float speed;

		AnimationTickArgs lastTick;
	};

};