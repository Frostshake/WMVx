#pragma once

#include <cmath>
#include "Math.h"

namespace core {

	class Vector3 {
	public:
		float x, y, z;

		Vector3(float x0 = 0.0f, float y0 = 0.0f, float z0 = 0.0f) : x(x0), y(y0), z(z0) {}

		Vector3(const Vector3& v) : x(v.x), y(v.y), z(v.z) {}

		Vector3(Vector3&&) = default;

		void reset() {
			x = y = z = 0.0f;
		}

		Vector3& operator= (const Vector3& v) {
			x = v.x;
			y = v.y;
			z = v.z;
			return *this;
		}


		Vector3 operator+ (const Vector3& v) const
		{
			Vector3 r(x + v.x, y + v.y, z + v.z);
			return r;
		}

		Vector3 operator- (const Vector3& v) const
		{
			Vector3 r(x - v.x, y - v.y, z - v.z);
			return r;
		}

		float operator* (const Vector3& v) const
		{
			return x * v.x + y * v.y + z * v.z;
		}

		Vector3 operator* (float d) const
		{
			Vector3 r(x * d, y * d, z * d);
			return r;
		}

		Vector3 operator/ (float d) const
		{
			Vector3 r(x / d, y / d, z / d);
			return r;
		}

		friend Vector3 operator* (float d, const Vector3& v)
		{
			return v * d;
		}

		// Cross Product
		Vector3 operator% (const Vector3& v) const
		{
			Vector3 r(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
			return r;
		}

		Vector3& operator+= (const Vector3& v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}

		Vector3& operator-= (const Vector3& v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}

		Vector3& operator*= (float d)
		{
			x *= d;
			y *= d;
			z *= d;
			return *this;
		}

		float lengthSquared() const
		{
			return x * x + y * y + z * z;
		}

		float length() const
		{
			return sqrtf(x * x + y * y + z * z);
		}

		Vector3& normalize()
		{
			float len = length();
			if (len == 0) {
				x = 0;
				y = 0;
				z = 0;
				return *this;
			}

			this->operator*= (1.0f / len);
			return *this;
		}

		Vector3 operator~ () const
		{
			Vector3 r(*this);
			r.normalize();
			return r;
		}

		static Vector3 yUpToZUp(const Vector3& v) {
			return Vector3(v.x, v.z, -v.y);
		}

		operator float* ()
		{
			return (float*)this;
		}

		float min() const {
			return std::min({ x,y, z });
		}


		float max() const {
			return std::max({ x,y,z });
		}
	};
}