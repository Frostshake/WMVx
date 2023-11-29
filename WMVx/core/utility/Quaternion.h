#pragma once

#include <cmath>
#include "Math.h"
#include "Vector4.h"


namespace core {

	class Matrix;

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

		static const Quaternion slerp(const float r, const Quaternion& v1, const Quaternion& v2);

		static const Quaternion lerp(const float r, const Quaternion& v1, const Quaternion& v2);

		Vector3 getHPB() const;

		Matrix toMat() const;

		Vector3 toEulerXYZ() const;

	};
}