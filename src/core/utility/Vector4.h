#pragma once

#include <cmath>
#include "Math.h"
#include "Vector3.h"

namespace core {

	class Vector4 {
	public:
		float x, y, z, w;

		Vector4(float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f, float w0 = 0.0f) : x(x0), y(y0), z(z0), w(w0) {}

		Vector4(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

		Vector4(Vector4&&) = default;

		Vector4(const Vector3& v, const float w0) : x(v.x), y(v.y), z(v.z), w(w0) {}

		Vector4& operator= (const Vector4& v) {
			x = v.x;
			y = v.y;
			z = v.z;
			w = v.w;
			return *this;
		}

		const Vector4 operator+ (const Vector4& v) const
		{
			Vector4 r(x + v.x, y + v.y, z + v.z, w + v.w);
			return r;
		}

		const Vector4 operator- (const Vector4& v) const
		{
			Vector4 r(x - v.x, y - v.y, z - v.z, w - v.w);
			return r;
		}

		const Vector4 operator* (float d) const
		{
			Vector4 r(x * d, y * d, z * d, w * d);
			return r;
		}

		friend Vector4 operator* (float d, const Vector4& v)
		{
			return v * d;
		}

		Vector4& operator+= (const Vector4& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}

		Vector4& operator-= (const Vector4& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}

		Vector4& operator*= (float d)
		{
			x *= d;
			y *= d;
			z *= d;
			w *= d;
			return *this;
		}

		float operator* (const Vector4& v) const
		{
			return x * v.x + y * v.y + z * v.z + w * v.w;
		}

		float lengthSquared() const
		{
			return x * x + y * y + z * z + w * w;
		}

		float length() const
		{
			return sqrtf(x * x + y * y + z * z + w * w);
		}

		Vector4& normalize()
		{
			this->operator*= (1.0f / length());
			return *this;
		}

		operator float* ()
		{
			return (float*)this;
		}

		Vector3 xyz() const
		{
			return Vector3(x, y, z);
		}
	};

}