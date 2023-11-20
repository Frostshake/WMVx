#pragma once
#include "../utility/Math.h"
#include <cstdint>
#include <vector>
#include "../utility/Quaternion.h"
#include "BFAM2Definitions.h"
#include "AnimationCommon.h"
#include "../filesystem/ChunkedFile.h"
#include "WOTLKAnimation.h"
#include "../filesystem/GameFileSystem.h"
#include "../utility/Memory.h"

namespace core {
	template <class T>
	class BFAAnimationBlock : public TimelineBasedAnimationBlock<T> {
	public:
		static BFAAnimationBlock<T> fromDefinition(const WOTLKAnimationBlockM2& definition, const std::vector<uint8_t>& buffer, const std::map<size_t, ChunkedFileInstance>& animFiles) {
			BFAAnimationBlock<T> anim_block;

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

					if (timestamp_headers[i].size == 0) {
						continue;
					}

					auto temp_times = std::vector<uint32_t>();
					temp_times.resize(timestamp_headers[i].size);

					if (animFiles.contains(i)) {
						const auto& fileInstance = animFiles.at(i);

						if (fileInstance.chunked.isChunked()) {
							const auto afsb_chunk = fileInstance.chunked.get("AFSB");
							const auto afm2_chunk = fileInstance.chunked.get("AFM2");

							if (afsb_chunk != nullptr) {
								fileInstance.file->read(temp_times.data(), sizeof(uint32_t) * timestamp_headers[i].size, afsb_chunk->offset + timestamp_headers[i].offset);
							}
							else if (afm2_chunk != nullptr) {
								fileInstance.file->read(temp_times.data(), sizeof(uint32_t) * timestamp_headers[i].size, afm2_chunk->offset + timestamp_headers[i].offset);
							}
							else {
								assert(false); //TODO
							}
						}
						else if(fileInstance.file->getFileSize() > timestamp_headers[i].offset){
							fileInstance.file->read(temp_times.data(), sizeof(uint32_t) * timestamp_headers[i].size, timestamp_headers[i].offset);
						}
					}
					else if (buffer.size() > timestamp_headers[i].offset) {
						memcpy_x(temp_times, buffer, timestamp_headers[i].offset, sizeof(uint32_t) * timestamp_headers[i].size);
					}
					else {
						assert(false);
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

					if (animFiles.contains(i)) {
						const auto& fileInstance = animFiles.at(i);
						if (fileInstance.chunked.isChunked()) {
							const auto afsb_chunk = fileInstance.chunked.get("AFSB");
							const auto afm2_chunk = fileInstance.chunked.get("AFM2");

							if (afsb_chunk != nullptr) {
								fileInstance.file->read(temp_keys.data(), sizeof(T) * key_headers[i].size, afsb_chunk->offset + key_headers[i].offset);
							} else if(afm2_chunk != nullptr) {
								fileInstance.file->read(temp_keys.data(), sizeof(T) * key_headers[i].size, afm2_chunk->offset + key_headers[i].offset);
							}
							else {
								assert(false); //TODO
							}
						}
						else if(fileInstance.file->getFileSize() > key_headers[i].offset) {
							fileInstance.file->read(temp_keys.data(), sizeof(T) * key_headers[i].size, key_headers[i].offset);
						}

					}
					else if (buffer.size() > key_headers[i].offset) {
						memcpy_x(temp_keys, buffer, key_headers[i].offset, sizeof(T) * key_headers[i].size);
					}
					else {
						assert(false);
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
	using BFAAnimated = WOTLKAnimated<T, D, Conv>;
}