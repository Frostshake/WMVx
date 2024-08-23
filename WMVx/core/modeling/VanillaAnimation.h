#pragma once
#include "../utility/Math.h"
#include <cstdint>
#include <vector>
#include "../utility/Quaternion.h"
#include "VanillaM2Definitions.h"
#include "AnimationCommon.h"
#include "../filesystem/GameFileSystem.h"
#include "../utility/Memory.h"

namespace core {

	template <class T>
	class VanillaAnimationBlock : public RangeBasedAnimationBlock<T> {
	public:
		static VanillaAnimationBlock<T>  fromDefinition(const AnimationBlockM2Legacy& definition, const std::vector<uint8_t>& buffer) {
			VanillaAnimationBlock<T> anim_block;
			anim_block.interpolationType = definition.interpolationType;
			anim_block.globalSequence = definition.globalSequence;

			assert(definition.timestamps.size == definition.keys.size);

			if (definition.ranges.size) {
				anim_block.ranges.resize(definition.ranges.size);
				memcpy_x(anim_block.ranges, buffer, definition.ranges.offset, sizeof(AnimationRange) * definition.ranges.size);
			}

			if (definition.timestamps.size) {
				anim_block.timestamps.resize(definition.timestamps.size);
				memcpy_x(anim_block.timestamps, buffer, definition.timestamps.offset, sizeof(uint32_t) * definition.timestamps.size);
			}

			if (definition.keys.size) {
				anim_block.keys.resize(definition.keys.size);
				memcpy_x(anim_block.keys, buffer, definition.keys.offset, sizeof(T) * (definition.keys.size));
			}

			return anim_block;
		}
	};

	
};