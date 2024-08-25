#pragma once

#include "../utility/Quaternion.h"
#include "M2Defintions.h"
#include <memory>
#include <map>
#include <span>
#include <vector>

namespace core {

	struct AnimationTickArgs {
		AnimationTickArgs(uint32_t frame = 0, uint32_t delta = 0, uint64_t time = 0) {
			currentFrame = frame;
			deltaTime = delta;
			absoluteTime = time;
		}
		AnimationTickArgs(AnimationTickArgs&&) = default;

		uint32_t currentFrame;
		uint32_t deltaTime;
		uint64_t absoluteTime;
	};

	enum Interpolation {
		INTERPOLATION_NONE,
		INTERPOLATION_LINEAR,
		INTERPOLATION_HERMITE,
		INTERPOLATION_BEZIER
	};


	template<typename T>
	class AnimatedValue {
	public:
		virtual Interpolation getType() const = 0;
		virtual bool uses(size_t animation_index) const = 0;
		virtual T getValue(size_t animation_index, const AnimationTickArgs& tick) const = 0;
	};

	//legacy type for use with RangeBasedAnimation
	struct AnimationRange {
		uint32_t start;
		uint32_t end;
	};

	// legacy animation blocks have a single time line, with ranges for each animation
	template <class T>
	class RangeBasedAnimationBlock {
	public:
		uint16_t interpolationType;
		int16_t globalSequence;
		std::vector<AnimationRange> ranges;
		std::vector<uint32_t> timestamps;
		std::vector<T> keys;

		template<M2_VER_RANGE R>
		static RangeBasedAnimationBlock<T> fromDefinition(const AnimationBlockM2<R>& definition, const std::span<uint8_t> buffer, const std::map<size_t, ChunkedFile2>& animFiles) {
			RangeBasedAnimationBlock<T> anim_block;

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

	// modern animation blocks have multiple timelines
	template <class T>
	class TimelineBasedAnimationBlock {
	public:
		uint16_t interpolationType;
		int16_t globalSequence;
		std::vector<std::vector<uint32_t>> timestamps;
		std::vector<std::vector<T>> keys;

		template<M2_VER_RANGE R>
		static TimelineBasedAnimationBlock<T> fromDefinition(const AnimationBlockM2<R>& definition, const std::span<uint8_t> buffer, const std::map<size_t, ChunkedFile2>& animFiles) {
			TimelineBasedAnimationBlock<T> anim_block;

			anim_block.interpolationType = definition.interpolationType;
			anim_block.globalSequence = definition.globalSequence;

			assert(definition.timestamps.size == definition.keys.size);

			auto load_data = [&](const M2Array& def, auto& dest) {
				using dest_val_t = decltype(dest)::element_type::element_type;

				if (def.size) {
					dest.reserve(def.size);

					const std::span<AnimationBlockHeader> headers(
						(AnimationBlockHeader*)(buffer.data() + def.offset), 
						def.size
					);

					size_t header_index = -1;
					for (const auto& header : headers) {
						header_index++;

						if (header.size == 0) {
							continue;
						}

						const auto read_size = sizeof(dest_val_t) * header.size;
						std::vector<dest_val_t> temp;
						temp.resize(header.size);

						const auto animFile = animFiles.find(header_index);
						if (animFile != animFiles.end()) {
							assert(animFile->second.file);
							if (animFile->second.isChunked()) {

								const auto afsb = animFile->second.chunks.find(Signatures::AFSB);
								const auto afm2 = animFile->second.chunks.find(Signatures::AFM2);
								if (afsb != animFile->second.chunks.end()) {
									animFile->second.file->read(temp.data(), read_size, afsb->second.offset + header.offset);
								}
								else if (afm2 != animFile->second.chunks.end()) {
									animFile->second.file->read(temp.data(), read_size, afm2->second.offset + header.offset);
								}
								else {
									assert(false);	//shouldnt happen
									continue;
								}
							}
							else if (animFile->second.file->getFileSize() >= (header.offset + read_size)) {
								animFile->second.file->read(temp.data(), read_size, header.offset);
							}
							else {
								assert(false);	//shouldnt happen
								continue;
							}
						}
						else if (buffer.size() >= (header.offset + read_size)) {
							memcpy_x(temp.data(), buffer, header.offset, read_size);
						}
						else {
							assert(false);	//shouldnt happen
							continue;
						}
						
						dest[header_index] = std::move(temp);
					}
				}

				assert(dest.size() == def.size);
			};

			load_data(definition.timestamps, anim_block.timestamps);
			load_data(definition.keys, anim_block.keys);

			return anim_block;
		}
	};

	template<class T, M2_VER_RANGE R>
	using AnimationBlock = std::conditional_t<M2_VER_CONDITION_AFTER<M2_VER_WOTLK>::eval(R), TimelineBasedAnimationBlock<T>, RangeBasedAnimationBlock<T>>;


	// interpolation functions
	template<class T>
	inline T interpolate(const float r, const T& v1, const T& v2)
	{
		return static_cast<T>(v1 * (1.0f - r) + v2 * r);
	}

	template<class T>
	inline T interpolateHermite(const float r, const T& v1, const T& v2, const T& in, const T& out)
	{
		// basis functions
		float h1 = 2.0f * r * r * r - 3.0f * r * r + 1.0f;
		float h2 = -2.0f * r * r * r + 3.0f * r * r;
		float h3 = r * r * r - 2.0f * r * r + r;
		float h4 = r * r * r - r * r;

		// interpolation
		return static_cast<T>(v1 * h1 + v2 * h2 + in * h3 + out * h4);
	}


	template<class T>
	inline T interpolateBezier(const float r, const T& v1, const T& v2, const T& in, const T& out)
	{
		float InverseFactor = (1.0f - r);
		float FactorTimesTwo = r * r;
		float InverseFactorTimesTwo = InverseFactor * InverseFactor;
		// basis functions
		float h1 = InverseFactorTimesTwo * InverseFactor;
		float h2 = 3.0f * r * InverseFactorTimesTwo;
		float h3 = 3.0f * FactorTimesTwo * InverseFactor;
		float h4 = FactorTimesTwo * r;

		// interpolation
		return static_cast<T>(v1 * h1 + v2 * h2 + in * h3 + out * h4);
	}

	// "linear" interpolation for quaternions should be slerp by default
	template<>
	inline Quaternion interpolate<Quaternion>(const float r, const Quaternion& v1, const Quaternion& v2)
	{
		return Quaternion::slerp(r, v1, v2);
	}


	template<class T>
	T lifeRamp(float life, float mid, const T& a, const T& b, const T& c)
	{
		if (life <= mid)
			return interpolate<T>(life / mid, a, b);
		else
			return interpolate<T>((life - mid) / (1.0f - mid), b, c);
	}


	//TODO messy code - tidy or replace - this has been copied directly from WMV - tidy

	template <class T>
	class Identity {
	public:
		static const T& conv(const T& t)
		{
			return t;
		}
	};

	//TODO CHECK WHICH VERSIONS PACK_QUATERNION / conv WORKS WITH
	struct PACK_QUATERNION {
		int16_t x, y, z, w;
	};

	class Quat16ToQuat32 {
	public:
		static const Quaternion conv(const PACK_QUATERNION t)
		{
			return Quaternion(
				float(t.x < 0 ? t.x + 32768 : t.x - 32767) / 32767.0f,
				float(t.y < 0 ? t.y + 32768 : t.y - 32767) / 32767.0f,
				float(t.z < 0 ? t.z + 32768 : t.z - 32767) / 32767.0f,
				float(t.w < 0 ? t.w + 32768 : t.w - 32767) / 32767.0f);
		}
	};

	// Convert opacity values stored as shorts to floating point
	// I wonder why Blizzard decided to save 2 bytes by doing this
	class ShortToFloat {
	public:
		static const float conv(const short t)
		{
			return t / 32767.0f;
		}
	};


#define	MAX_ANIMATED	500


	template<typename T>
	class TimelineBasedAnimatedValue : public AnimatedValue<T> {
	public:
		TimelineBasedAnimatedValue() = default;
		TimelineBasedAnimatedValue(TimelineBasedAnimatedValue&&) = default;
		virtual ~TimelineBasedAnimatedValue() {}

		Interpolation getType() const override {
			return interpolationType;
		}

		bool uses(size_t animation_index) const override {

			if (globalSequence > -1) {
				animation_index = 0;
			}

			auto found = data.find(animation_index);
			if (found != data.end()) {
				//ideally map entries are not created with zero entries, however its not guarenteed
				return found->second.size() > 0;
			}

			return false;
		}

		T getValue(size_t animation_index, const AnimationTickArgs& tick) const override {
		
			auto time = tick.currentFrame;

			// obtain a time value and a data range
			if (globalSequence > -1 && globalSequence < globals->size()) {

				const auto& global_match = globals->at(globalSequence);

				if (!global_match) {
					return T();
				}

				if (global_match == 0) {
					time = 0;
				}
				else {
					time = tick.absoluteTime % global_match;
				}

				animation_index = 0;
			}

			const auto data_match = data.find(animation_index);
			const auto times_match = times.find(animation_index);

			if (data_match != data.end() && times_match != times.end() &&
				data_match->second.size() > 1 && times_match->second.size() > 1) {

				auto compute = [&](size_t pos, size_t pos2, float r) {
					switch (interpolationType) {
					case INTERPOLATION_NONE:
						return data_match->second[pos];
					case INTERPOLATION_LINEAR:
						return interpolate<T>(r, data_match->second[pos], data_match->second[pos2]);
					case INTERPOLATION_HERMITE:
						// INTERPOLATION_HERMITE is only used in cameras afaik?
						return interpolateHermite<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos2], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					case INTERPOLATION_BEZIER:
						//Is this used ingame or only by custom models?
						return interpolateBezier<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos2], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					default:
						//this shouldn't appear!
						return data_match->second[pos];
						
					}
				};

				float r = 1.0f;
				size_t pos = 0;
				const size_t max_time = times_match->second.back();

				//if (max_time > 0)
				//	time %= max_time; // I think this might not be necessary?
				if (time > max_time) {
					pos = times_match.size() - 1;

					return compute(pos, pos, r);
				}
				else {
					for (size_t i = 0; i < times_match->second.size() - 1; i++) {
						if (time >= times_match->second[i] && time < times_match->second[i + 1]) {
							pos = i;
							break;
						}
					}
					size_t t1 = times_match->second[pos];
					size_t t2 = times_match->second[pos + 1];
					r = (time - t1) / (float)(t2 - t1);

					return compute(pos, pos + 1, r);
				}

			}
			else if(data_match != data.end() && data_match->second.size() > 0) {
				return data.at(animation_index)[0];
			}

			return T();
		}

		template<typename D = T, class Conv = Identity<T>>
		static TimelineBasedAnimatedValue<T> make(TimelineBasedAnimationBlock<D>&& block, std::shared_ptr<std::vector<uint32_t>> globalSequences, auto fix_fn) {
			TimelineBasedAnimatedValue<T> result;
			result.globals = globalSequences;
			result.interpolationType = block.interpolationType;
			result.globalSequence = block.globalSequence;

			if (result.globalSequence != -1) {
				if (!globalSequences->size()) {
					return result;
				}
			}

			assert(block.timestamps.size() == block.keys.size());

			// times
			if (block.timestamps.size() != block.keys.size()) {
				return result;
			}
			else if (block.timestamps.size() == 0) {
				return result;
			}

			for (size_t j = 0; j < block.timestamps.size(); j++) {
				result.times.emplace(j, std::move(block.timestamps[j]));
			}

			////keys
			for (size_t j = 0; j < block.keys.size(); j++) {

				switch (result.type) {
					case INTERPOLATION_NONE:
					case INTERPOLATION_LINEAR:
					{

						std::for_each(block.keys[j].begin(), block.keys[j].end(), [](auto& val) {
							val = fix_fn(Conv::conv(val));
							});

						result.data.emplace(j, std::move(block.keys[j]));

					}
					break;
					case INTERPOLATION_HERMITE:
					case INTERPOLATION_BEZIER:
					{
						//TODO implement
						assert(false);
						throw 1;
						//for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
						//	data[j].push_back(Conv::conv(keys[i * 3]));
						//	in[j].push_back(Conv::conv(keys[i * 3 + 1]));
						//	out[j].push_back(Conv::conv(keys[i * 3 + 2]));
						//}
					}
					break;
				}
			}
		

			if (result.times.size() > 0) {
				assert(result.times.rbegin()->first <= (MAX_ANIMATED - 1));
			}
			if (result.data.size() > 0) {
				assert(result.data.rbegin()->first <= (MAX_ANIMATED - 1));
			}

			return result;
		}

	protected:
		int32_t interpolationType;
		int32_t globalSequence;
		std::shared_ptr<std::vector<uint32_t>> globals;

		std::map<size_t, std::vector<size_t>> times;
		std::map<size_t, std::vector<T>> data;

		// for nonlinear interpolations:
		std::map<size_t, std::vector<T>> in;
		std::map<size_t, std::vector<T>> out;
	};

	template<typename T>
	class RangeBasedAnimatedValue : public AnimatedValue<T> {
	public:
		RangeBasedAnimatedValue() = default;
		RangeBasedAnimatedValue(RangeBasedAnimatedValue&&) = default;
		virtual ~RangeBasedAnimatedValue() {}

		Interpolation getType() const override {
			return interpolationType;
		}

		bool uses(size_t animation_index) const override {

			if (globalSequence > -1) {
				animation_index = 0;
			}

			return ranges.size() > animation_index;
		}

		T getValue(size_t animation_index, const AnimationTickArgs& tick) const override {
			auto time = tick.currentFrame;
			// obtain a time value and a data range
			if (globalSequence > -1 && globalSequence < globals->size()) {

				const auto& global_match = globals->at(globalSequence);

				if (!global_match) {
					return T();
				}

				if (global_match == 0) {
					time = 0;
				}
				else {
					time = tick.absoluteTime % global_match;
				}

				animation_index = 0;
			}

			if (ranges.size() > animation_index) {

				const auto& range = ranges.at(animation_index);
				float r = 1.0f;
				const size_t max_time = timestamps[range.end];
				const size_t start_time = timestamps[range.start];
				size_t pos = 0;

				time = start_time + time;

			//	// if (max_time > 0)
			//	//	time %= max_time; // I think this might not be necessary?
				if (time > max_time) {
					pos = timestamps.size() - 1;
					//TODO handle types

					return data[pos]; //interpolate<T>(r, data[pos], data[pos]);
				}
				else {
					//TODO can this be limited t range.start and range.end?
					for (size_t i = 0; i < timestamps.size() - 1; i++) {
						if (time >= timestamps[i] && time < timestamps[i + 1]) {
							pos = i;
							break;
						}
					}

					size_t t1 = timestamps[pos];
					size_t t2 = timestamps[pos + 1];
					r = (time - t1) / (float)(t2 - t1);

					switch (interpolationType) {
					case INTERPOLATION_NONE:
						return data[pos];
						break;
					case INTERPOLATION_LINEAR:
						return interpolate<T>(r, data[pos], data[pos + 1]);
						break;
					case INTERPOLATION_HERMITE:
					case INTERPOLATION_BEZIER:
						//TODO implement
						assert(false);
						throw 1;
						break;
					}
				}
			}

			return T(); 
		}

		template<typename D = T, class Conv = Identity<T>>
		static RangeBasedAnimatedValue<T> make(RangeBasedAnimationBlock<D>&& block, std::shared_ptr<std::vector<uint32_t>> globalSequences, auto fix_fn) {
			RangeBasedAnimatedValue<T> result;
			result.globals = globalSequences;
			result.interpolationType = block.interpolationType;
			result.globalSequence = block.globalSequence;

			if (result.globalSequence != -1) {
				if (!globalSequences->size()) {
					return result;
				}
			}

			assert(block.timestamps.size() == block.keys.size());

			// times
			if (block.timestamps.size() != block.keys.size()) {
				return result;
			}
			else if (block.timestamps.size() == 0) {
				return result;
			}

			result.ranges = std::move(block.ranges);
			result.timestamps = std::move(block.timestamps);

			switch(result.interpolationType) {
				case INTERPOLATION_NONE:
				case INTERPOLATION_LINEAR:
				{
					std::for_each(block.keys.begin(), block.keys.end(), [](auto& val) {
						val = fix_fn(Conv::conv(val));
						});

					result.data = std::move(block.keys);
				}
				break;
				case INTERPOLATION_HERMITE:
				case INTERPOLATION_BEZIER:
				{
					//TODO implement
					assert(false);
					throw 1;
					//for (size_t i = 0; i < pHeadKeys->nEntrys; i++) {
					//	data[j].push_back(Conv::conv(keys[i * 3]));
					//	in[j].push_back(Conv::conv(keys[i * 3 + 1]));
					//	out[j].push_back(Conv::conv(keys[i * 3 + 2]));
					//}
				}
				break;
			}

			return result;
		}

	protected:
		int32_t interpolationType;
		int32_t globalSequence;
		std::shared_ptr<std::vector<uint32_t>> globals;

		std::vector<AnimationRange> ranges;
		std::vector<uint32_t> timestamps;
		std::vector<T> data;
	};


	template <class T, class D = T, class Conv = Identity<T> >
	class StandardAnimated : public AnimatedValue<T> {
	public:

		StandardAnimated() = default;
		StandardAnimated(StandardAnimated&&) = default;
		virtual ~StandardAnimated() {}

		virtual Interpolation getType() const override {
			return (Interpolation)type;
		}

		bool uses(size_t animation_index) const override
		{
			if (seq > -1) {
				animation_index = 0;
			}

			//ideally map entries are not created with zero entries, however its not guarenteed
			return data.contains(animation_index) && data.at(animation_index).size() > 0;
		}


		T getValue(size_t animation_index, const AnimationTickArgs& tick) const override
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
				}
				else {
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
					}
					else if (type == INTERPOLATION_HERMITE) {
						// INTERPOLATION_HERMITE is only used in cameras afaik?
						return interpolateHermite<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					}
					else if (type == INTERPOLATION_BEZIER) {
						//Is this used ingame or only by custom models?
						return interpolateBezier<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					}
					else {
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
					}
					else if (type == INTERPOLATION_HERMITE) {
						// INTERPOLATION_HERMITE is only used in cameras afaik?
						return interpolateHermite<T>(r, data.at(animation_index)[pos], data.at(animation_index)[pos + 1], in.at(animation_index)[pos], out.at(animation_index)[pos]);
					}
					else if (type == INTERPOLATION_BEZIER) {
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
			type = b.interpolationType;
			seq = b.globalSequence;
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


	template <class T, class D = T, class Conv = Identity<T> >
	class LegacyAnimated : public AnimatedValue<T> {
	public:
		LegacyAnimated() = default;
		LegacyAnimated(LegacyAnimated&&) = default;
		virtual ~LegacyAnimated() {}

		virtual Interpolation getType() const override {
			return (Interpolation)type;
		}

		bool uses(size_t animation_index) const override
		{
			if (seq > -1) {
				animation_index = 0;
			}

			return ranges.size() > animation_index;
		}


		T getValue(size_t animation_index, const AnimationTickArgs& tick) const override
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
				}
				else {
					time = globalTime % globals->at(seq);
				}

				animation_index = 0;
			}

			if (ranges.size() > animation_index) {
				const auto& range = ranges.at(animation_index);

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
					}
					else {
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
			type = b.interpolationType;
			seq = b.globalSequence;

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