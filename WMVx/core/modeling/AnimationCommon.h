#pragma once

#include "../utility/Quaternion.h"
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


	//legacy type for use with RangeBasedAnimation
	struct AnimationRange {
		uint32_t start;
		uint32_t end;
	};

	// legacy animation blocks have a single time line, with ranges for each animation
	template <class T>
	class RangeBasedAnimationBlock {
	public:
		uint16_t type;
		int16_t sequence;
		std::vector<AnimationRange> ranges;
		std::vector<uint32_t> timestamps;
		std::vector<T> keys;
	};

	// modern animation blocks have multiple timelines
	template <class T>
	class TimelineBasedAnimationBlock {
	public:
		uint16_t type;
		int16_t sequence;
		std::vector<std::vector<uint32_t>> timestamps;
		std::vector<std::vector<T>> keys;
	};

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

	enum Interpolation {
		INTERPOLATION_NONE,
		INTERPOLATION_LINEAR,
		INTERPOLATION_HERMITE,
		INTERPOLATION_BEZIER
	};

#define	MAX_ANIMATED	500

};