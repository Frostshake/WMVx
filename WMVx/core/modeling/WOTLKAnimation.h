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
		static WOTLKAnimationBlock<T> fromDefinition(const AnimationBlockM2<M2_VER_RANGE::EXACT(M2_VER_WOTLK)>& definition, const std::vector<uint8_t>& buffer, const std::map<size_t, ArchiveFile*> animFiles) {
			WOTLKAnimationBlock<T> anim_block;

			anim_block.interpolationType = definition.interpolationType;
			anim_block.globalSequence = definition.globalSequence;

			auto timestamp_headers = std::vector<AnimationBlockHeader>();
			auto key_headers = std::vector<AnimationBlockHeader>();

			assert(definition.timestamps.size == definition.keys.size);

			if (definition.timestamps.size) {
				anim_block.timestamps.resize(definition.timestamps.size);
				timestamp_headers.resize(definition.timestamps.size);

				memcpy_x(timestamp_headers, buffer, definition.timestamps.offset, sizeof(AnimationBlockHeader) * definition.timestamps.size);

				for (auto i = 0; i < timestamp_headers.size(); i++) {

					if (timestamp_headers[i].size == 0) {
						continue;
					}

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

					if (key_headers[i].size == 0) {
						continue;
					}

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


}