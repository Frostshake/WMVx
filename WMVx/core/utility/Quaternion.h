#pragma once

#include <cmath>
#include "Math.h"
#include "Vector4.h"

namespace core {

	class Quaternion : public Vector4 {
	public:
		Quaternion(float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f, float w0 = 1.0f) : Vector4(x0, y0, z0, w0) {}

		Quaternion(const Vector4& v) : Vector4(v) {}

		Quaternion(const Quaternion&) = default;

		Quaternion(Quaternion&&) = default;

		Quaternion(const Vector3& v, const float w0) :Vector4(v, w0) {}

		Quaternion& operator= (const Quaternion& q) {
			x = q.x;
			y = q.y;
			z = q.z;
			w = q.w;
			return *this;
		}

		static const Quaternion slerp(const float r, const Quaternion& v1, const Quaternion& v2)
		{
			// SLERP
			float dot = v1 * v2;

			if (fabs(dot) > 0.9995f) {
				// fall back to LERP
				return Quaternion::lerp(r, v1, v2);
			}

			float a = acosf(dot) * r;
			Quaternion q = (v2 - v1 * dot);
			q.normalize();

			return v1 * cosf(a) + q * sinf(a);
		}

		static const Quaternion lerp(const float r, const Quaternion& v1, const Quaternion& v2)
		{
			return v1 * (1.0f - r) + v2 * r;
		}

		Vector3 GetHPB()
		{
			Vector3 hpb;
			hpb.x = atan2(2 * (x * z + y * w), 1 - 2 * (x * x + y * y));
			float sp = 2 * (x * w - y * z);
			if (sp < -1) sp = -1;
			else if (sp > 1) sp = 1;
			hpb.y = asin(sp);
			hpb.z = atan2(2 * (x * y + z * w), 1 - 2 * (x * x + z * z));

			return hpb;
		}

	};
}