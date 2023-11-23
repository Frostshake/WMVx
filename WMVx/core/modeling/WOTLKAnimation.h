#pragma once
#include "../utility/Math.h"
#include <cstdint>
#include <vector>
#include <map>
#include "../utility/Quaternion.h"
#include "WOTLKM2Definitions.h"
#include "AnimationCommon.h"
#include "../filesystem/GameFileSystem.h"
#include "../utility/Memory.h"

namespace core {
	template <class T>
	class WOTLKAnimationBlock : public TimelineBasedAnimationBlock<T> {
	public:
		static WOTLKAnimationBlock<T> fromDefinition(const WOTLKAnimationBlockM2& definition, const std::vector<uint8_t>& buffer, const std::map<size_t, ArchiveFile*> animFiles) {
			WOTLKAnimationBlock<T> anim_block;

			anim_block.type = definition.type;
			anim_block.sequence = definition.sequence;

			auto timestamp_headers = std::vector<AnimationBlockHeader>();
			auto key_headers = std::vector<AnimationBlockHeader>();

			assert(definition.timestamps.size == definition.keys.size);

			if (definition.timestamps.size) {
				anim_block.timestamps.resize(definition.timestamps.size);
				timestamp_headers.resize(definition.timestamps.size);

				memcpy_x(timestamp_headers, buffer, definition.timestamps.offset, sizeof(AnimationBlockHeader) * definition.timestamps.size);

				for (auto i = 0; i < timestamp_headers.size(); i++) {

					auto temp_times = std::vector<uint32_t>();
					temp_times.resize(timestamp_headers[i].size);

					if (animFiles.contains(i) &&
						animFiles.at(i)->getFileSize() > timestamp_headers[i].offset) {
						animFiles.at(i)->read(temp_times.data(), sizeof(uint32_t) * timestamp_headers[i].size, timestamp_headers[i].offset);
					}
					else if (buffer.size() > timestamp_headers[i].offset) {
						memcpy_x(temp_times, buffer, timestamp_headers[i].offset, sizeof(uint32_t) * timestamp_headers[i].size);
					}
					else {
						continue;
					}

					for (auto j = 0; j < temp_times.size(); j++) {
						anim_block.timestamps[i].push_back(temp_times[j]);
					}
				}

				assert(anim_block.timestamps.size() == timestamp_headers.size());
			}

			if (definition.keys.size) {
				anim_block.keys.resize(definition.keys.size);
				key_headers.resize(definition.keys.size);

				memcpy_x(key_headers, buffer, definition.keys.offset, sizeof(AnimationBlockHeader) * definition.keys.size);

				for (auto i = 0; i < key_headers.size(); i++) {
					auto temp_keys = std::vector<T>();
					temp_keys.resize(key_headers[i].size);

					if (animFiles.contains(i) &&
						animFiles.at(i)->getFileSize() > key_headers[i].offset) {
						animFiles.at(i)->read(temp_keys.data(), sizeof(T) * key_headers[i].size, key_headers[i].offset);
					}
					else if (buffer.size() > key_headers[i].offset) {
						memcpy_x(temp_keys, buffer, key_headers[i].offset, sizeof(T) * key_headers[i].size);
					}
					else {
						continue;
					}

					for (auto j = 0; j < temp_keys.size(); j++) {
						anim_block.keys[i].push_back(temp_keys[j]);
					}
				}

				assert(anim_block.keys.size() == key_headers.size());
			}

			assert(timestamp_headers.size() == key_headers.size());

			return anim_block;
		}
	};



	template <class T, class D = T, class Conv = Identity<T> >
	class WOTLKAnimated {
	public:

		bool uses(size_t animation_index) const
		{
			if (seq > -1) {
				animation_index = 0;
			}

			//ideally map entries are not created with zero entries, however its not guarenteed
			return data.contains(animation_index) && data.at(animation_index).size() > 0;
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

			if (data.contains(animation_index) && data.at(animation_index).size() > 1 &&
				times.contains(animation_index) && times.at(animation_index).size() > 1) {
				size_t t1, t2;
				size_t pos = 0;
				float r = 1.0f;
				size_t max_time = times.at(animation_index).back();
				//if (max_time > 0)
				//	time %= max_time; // I think this might not be necessary?
				if (time > max_time) {
					pos = times.at(animation_index).size() - 1;

					//TODO not sure the argements are correct for the interpolate functions - they all same value!?
					if (type == INTERPOLATION_NONE) {
						return data.at(animation_index)[pos];
					}
					else if (type == INTERPOLATION_LINEAR) {
						return interpolate<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos]);
					} else if (type == INTERPOLATION_HERMITE) {
						// INTERPOLATION_HERMITE is only used in cameras afaik?
						return interpolateHermite<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					} else if (type == INTERPOLATION_BEZIER) {
						//Is this used ingame or only by custom models?
						return interpolateBezier<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					} else { 
						//this shouldn't appear!
						return data.at(animation_index)[pos];
					}
				}
				else {
					for (size_t i = 0; i < times.at(animation_index).size() - 1; i++) {
						if (time >= times.at(animation_index)[i] && time < times.at(animation_index)[i + 1]) {
							pos = i;
							break;
						}
					}
					t1 = times.at(animation_index)[pos];
					t2 = times.at(animation_index)[pos + 1];
					r = (time - t1) / (float)(t2 - t1);

					if (type == INTERPOLATION_NONE) {
						return data.at(animation_index)[pos];
					}
					else if (type == INTERPOLATION_LINEAR) {
						return interpolate<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos + 1]);
					} else if (type == INTERPOLATION_HERMITE) {
						// INTERPOLATION_HERMITE is only used in cameras afaik?
						return interpolateHermite<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos + 1], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					} else if (type == INTERPOLATION_BEZIER) {
						//Is this used ingame or only by custom models?
						return interpolateBezier<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos + 1], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					}
					else {
						//this shouldn't appear!
						return data.at(animation_index)[pos];
					}
				}
			}
			else {
				//		// default value
				if (!data.contains(animation_index) || data.at(animation_index).size() == 0) {
					return T();
				}
				else {
					return data.at(animation_index)[0];
				}
			}

			return T();
		}

		void init(const TimelineBasedAnimationBlock<D>& b, std::shared_ptr<std::vector<uint32_t>> globalSequences)
		{
			globals = globalSequences;
			type = b.type;
			seq = b.sequence;
			if (seq != -1) {
				if (!globalSequences->size()) {
					return;
				}
			}

			// times
			if (b.timestamps.size() != b.keys.size()) {
				return;
			}

			sizes = b.timestamps.size();
			if (b.timestamps.size() == 0) {
				return;
			}

			for (size_t j = 0; j < b.timestamps.size(); j++) {
				for (auto x = 0; x < b.timestamps[j].size(); x++) {
					times[j].push_back(b.timestamps[j][x]);
				}
			}


			// keyframes
			for (size_t j = 0; j < b.keys.size(); j++) {
				const auto& keys = b.keys[j];

				switch (type) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
					for (auto i = 0; i < keys.size(); i++) {
						data[j].push_back(Conv::conv(keys[i]));
					}
					break;
				case INTERPOLATION_HERMITE:
					assert(false);
					throw 1;
					//for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
					//	data[j].push_back(Conv::conv(keys[i * 3]));
					//	in[j].push_back(Conv::conv(keys[i * 3 + 1]));
					//	out[j].push_back(Conv::conv(keys[i * 3 + 2]));
					//}
					break;
				case INTERPOLATION_BEZIER:
					assert(false);
					throw 2;
					//for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
					//	data[j].push_back(Conv::conv(keys[i * 3]));
					//	in[j].push_back(Conv::conv(keys[i * 3 + 1]));
					//	out[j].push_back(Conv::conv(keys[i * 3 + 2]));
					//}
					break;
				}
			}

			if (times.size() > 0) {
				assert(times.rbegin()->first <= (MAX_ANIMATED - 1));
			}
			if (data.size() > 0) {
				assert(data.rbegin()->first <= (MAX_ANIMATED - 1));
			}
		}

		void fix(T fixfunc(const T&))
		{
			switch (type) {
			case INTERPOLATION_NONE:
			case INTERPOLATION_LINEAR:
				for (size_t i = 0; i < sizes; i++) {
					for (size_t j = 0; j < data[i].size(); j++) {
						data[i][j] = fixfunc(data[i][j]);
					}
				}
				break;
			case INTERPOLATION_HERMITE:
				assert(false);
				throw 1;
				//		for (size_t i = 0; i < sizes; i++) {
				//			for (size_t j = 0; j < data[i].size(); j++) {
				//				data[i][j] = fixfunc(data[i][j]);
				//				in[i][j] = fixfunc(in[i][j]);
				//				out[i][j] = fixfunc(out[i][j]);
				//			}
				//		}

				break;
			case INTERPOLATION_BEZIER:
				assert(false);
				throw 2;
				//		for (size_t i = 0; i < sizes; i++) {
				//			for (size_t j = 0; j < data[i].size(); j++) {
				//				data[i][j] = fixfunc(data[i][j]);
				//				in[i][j] = fixfunc(in[i][j]);
				//				out[i][j] = fixfunc(out[i][j]);
				//			}
				//		}
				break;
			}
		}

	protected:
		int32_t type;
		int32_t seq;
		std::shared_ptr<std::vector<uint32_t>> globals;

		std::map<size_t, std::vector<size_t>> times;
		std::map<size_t, std::vector<T>> data;

		// for nonlinear interpolations:
		std::map<size_t, std::vector<T>> in;
		std::map<size_t, std::vector<T>> out;

		size_t sizes; // for fix function

	};
}