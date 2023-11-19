#pragma once
#include "../utility/Math.h"
#include <cstdint>
#include <vector>
#include "../utility/Quaternion.h"
#include "VanillaM2Definitions.h"
#include "AnimationCommon.h"
#include "../filesystem/GameFileSystem.h"

namespace core {

	template <class T>
	class VanillaAnimationBlock : public RangeBasedAnimationBlock<T> {
	public:
		static VanillaAnimationBlock<T>  fromDefinition(const VanillaAnimationBlockM2& definition, const std::vector<uint8_t>& buffer) {
			VanillaAnimationBlock<T> anim_block;
			anim_block.type = definition.type;
			anim_block.sequence = definition.sequence;

			assert(definition.timestamps.size == definition.keys.size);

			if (definition.ranges.size) {
				anim_block.ranges.resize(definition.ranges.size);

				memcpy(anim_block.ranges.data(),
					buffer.data() + definition.ranges.offset,
					sizeof(AnimationRange) * definition.ranges.size
				);
			}

			if (definition.timestamps.size) {
				anim_block.timestamps.resize(definition.timestamps.size);

				memcpy(anim_block.timestamps.data(),
					buffer.data() + definition.timestamps.offset,
					sizeof(uint32_t) * definition.timestamps.size
				);
			}

			if (definition.keys.size) {
				anim_block.keys.resize(definition.keys.size);

				memcpy(anim_block.keys.data(),
					buffer.data() + definition.keys.offset,
					sizeof(T) * (definition.keys.size)
				);
			}

			return anim_block;
		}
	};

	template <class T, class D = T, class Conv = Identity<T> >
	class VanillaAnimated {
	public:
		VanillaAnimated() = default;
		VanillaAnimated(VanillaAnimated&&) = default;
		virtual ~VanillaAnimated() {}

		bool uses(size_t animation_index) const
		{
			if (seq && seq > -1) {
				animation_index = 0;
			}

			return ranges.size() > animation_index;
		}


		T getValue(size_t animation_index, const AnimationTickArgs& tick) const
		{
			auto time = tick.currentFrame;
			auto globalTime = tick.absoluteTime;

			// obtain a time value and a data range
			if (seq > -1 && seq < globals->size()) {
				if (!globals->at(seq)) {
					return T();
				}

				if (globals->at(seq) == 0) {
					time = 0;
				} else {
					time = globalTime % globals->at(seq);
				}

				animation_index = 0;
			}

			if (ranges.size() > animation_index) {
				auto range = ranges.at(animation_index);

				size_t t1, t2;
				size_t pos = 0;
				float r = 1.0f;
				size_t max_time = timestamps[range.end];
				size_t start_time = timestamps[range.start];


				time = start_time + time;

				// if (max_time > 0)
				//	time %= max_time; // I think this might not be necessary?
				if (time > max_time) {
					pos = timestamps.size() - 1;
					//TODO handle types

					return interpolate<T>(r, data[pos], data[pos]);
				}
				else {
					for (size_t i = 0; i < timestamps.size() - 1; i++) {
						if (time >= timestamps[i] && time < timestamps[i + 1]) {
							pos = i;
							break;
						}
					}

					t1 = timestamps[pos];
					t2 = timestamps[pos + 1];
					r = (time - t1) / (float)(t2 - t1);

					if (type == INTERPOLATION_NONE) {
						return data[pos];
					}
					else if (type == INTERPOLATION_LINEAR) {
						return interpolate<T>(r, data[pos], data[pos + 1]);
					} else {
						//TODO handle other interpt types
					}

					return data[pos];
				}
			}

			return T();
		}

		void init(const RangeBasedAnimationBlock<D>& b, std::shared_ptr<std::vector<uint32_t>> globalSequences)
		{
			globals = globalSequences;
			type = b.type;
			seq = b.sequence;

			if (seq != -1) {
				if (!globalSequences->size()) {
					return;
				}
			}

			if (b.timestamps.size() != b.keys.size()) {
				return;
			}

			sizes = b.timestamps.size();
			if (b.timestamps.size() == 0) {
				return;
			}

			ranges = b.ranges;
			timestamps = b.timestamps;

			for (size_t j = 0; j < b.keys.size(); j++) {
				//TODO handle all types
				switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					data.push_back(Conv::conv(b.keys[j]));
					break;
				case INTERPOLATION_HERMITE:
					throw 1;	
					//for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
					//	data[j].push_back(Conv::conv(keys[i * 3]));
					//	in[j].push_back(Conv::conv(keys[i * 3 + 1]));
					//	out[j].push_back(Conv::conv(keys[i * 3 + 2]));
					//}
					break;
				case INTERPOLATION_BEZIER:
					throw 2; 
					//for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
					//	data[j].push_back(Conv::conv(keys[i * 3]));
					//	in[j].push_back(Conv::conv(keys[i * 3 + 1]));
					//	out[j].push_back(Conv::conv(keys[i * 3 + 2]));
					//}
					break;
				}
			}
		}

		void fix(T fixfunc(const T&))
		{
			switch (type) {
			case INTERPOLATION_NONE:
			case INTERPOLATION_LINEAR:
				for (size_t j = 0; j < data.size(); j++) {
					data[j] = fixfunc(data[j]);
				}
				break;
			case INTERPOLATION_HERMITE:
				assert(false);
				throw 1;
				break;
			case INTERPOLATION_BEZIER:
				assert(false);
				throw 2;
				break;
			}
		}

	protected:
		int32_t type;
		int32_t seq;
		std::shared_ptr<std::vector<uint32_t>> globals;

		std::vector<AnimationRange> ranges;
		std::vector<uint32_t> timestamps;
		std::vector<T> data;

		size_t sizes; // for fix function
	};

};