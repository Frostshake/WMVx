#pragma once

#include <cmath>
#include "Math.h"

namespace core {

	class Vector2 {
	public:
		float x, y;

		Vector2(float x0 = 0.0f, float y0 = 0.0f) : x(x0), y(y0) {}

		Vector2(const Vector2& v) : x(v.x), y(v.y) {}

		Vector2& operator= (const Vector2& v) {
			x = v.x;
			y = v.y;
			return *this;
		}

		Vector2 operator+ (const Vector2& v) const
		{
			Vector2 r(x + v.x, y + v.y);
			return r;
		}

		Vector2 operator- (const Vector2& v) const
		{
			Vector2 r(x - v.x, y - v.y);
			return r;
		}

		float operator* (const Vector2& v) const
		{
			return x * v.x + y * v.y;
		}

		Vector2 operator* (float d) const
		{
			Vector2 r(x * d, y * d);
			return r;
		}

		friend Vector2 operator* (float d, const Vector2& v)
		{
			return v * d;
		}

		Vector2& operator+= (const Vector2& v)
		{
			x += v.x;
			y += v.y;
			return *this;
		}

		Vector2& operator-= (const Vector2& v)
		{
			x -= v.x;
			y -= v.y;
			return *this;
		}

		Vector2& operator*= (float d)
		{
			x *= d;
			y *= d;
			return *this;
		}

		float lengthSquared() const
		{
			return x * x + y * y;
		}

		float length() const
		{
			return sqrtf(x * x + y * y);
		}

		Vector2& normalize()
		{
			this->operator*= (1.0f / length());
			return *this;
		}

		Vector2 operator~ () const
		{
			Vector2 r(*this);
			r.normalize();
			return r;
		}

		operator float* ()
		{
			return (float*)this;
		}

		float min() const {
			return std::min({ x,y });
		}

		float max() const {
			return std::max({ x,y });
		}
	};

}