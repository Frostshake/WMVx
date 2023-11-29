#pragma once

#include <cmath>
#include "Math.h"
#include "Vector3.h"
#include "Quaternion.h"

namespace core {

	class Matrix {
	public:
		float m[4][4];

		Matrix()
		{
		}

		Matrix(const Matrix& p)
		{
			for (size_t j = 0; j < 4; j++) {
				for (size_t i = 0; i < 4; i++) {
					m[j][i] = p.m[j][i];
				}
			}
		}

		Matrix(Matrix&&) = default;

		Matrix& operator= (const Matrix& p)
		{
			for (size_t j = 0; j < 4; j++) {
				for (size_t i = 0; i < 4; i++) {
					m[j][i] = p.m[j][i];
				}
			}
			return *this;
		}

		void zero();
		void unit();

		void translation(const Vector3& tr);
		static const Matrix newTranslation(const Vector3& tr);
		void scale(const Vector3& sc);
		static const Matrix newScale(const Vector3& sc);

		void quaternionRotate(const Quaternion& q);
		void QRotate(const Quaternion& q);
		static const Matrix newQuatRotate(const Quaternion& qr);

		Vector3 operator* (const Vector3& v) const
		{
			Vector3 o;
			o.x = m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3];
			o.y = m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3];
			o.z = m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3];
			return o;
		}

		Matrix operator* (const Matrix& p) const
		{
			Matrix o;
			o.m[0][0] = m[0][0] * p.m[0][0] + m[0][1] * p.m[1][0] + m[0][2] * p.m[2][0] + m[0][3] * p.m[3][0];
			o.m[0][1] = m[0][0] * p.m[0][1] + m[0][1] * p.m[1][1] + m[0][2] * p.m[2][1] + m[0][3] * p.m[3][1];
			o.m[0][2] = m[0][0] * p.m[0][2] + m[0][1] * p.m[1][2] + m[0][2] * p.m[2][2] + m[0][3] * p.m[3][2];
			o.m[0][3] = m[0][0] * p.m[0][3] + m[0][1] * p.m[1][3] + m[0][2] * p.m[2][3] + m[0][3] * p.m[3][3];

			o.m[1][0] = m[1][0] * p.m[0][0] + m[1][1] * p.m[1][0] + m[1][2] * p.m[2][0] + m[1][3] * p.m[3][0];
			o.m[1][1] = m[1][0] * p.m[0][1] + m[1][1] * p.m[1][1] + m[1][2] * p.m[2][1] + m[1][3] * p.m[3][1];
			o.m[1][2] = m[1][0] * p.m[0][2] + m[1][1] * p.m[1][2] + m[1][2] * p.m[2][2] + m[1][3] * p.m[3][2];
			o.m[1][3] = m[1][0] * p.m[0][3] + m[1][1] * p.m[1][3] + m[1][2] * p.m[2][3] + m[1][3] * p.m[3][3];

			o.m[2][0] = m[2][0] * p.m[0][0] + m[2][1] * p.m[1][0] + m[2][2] * p.m[2][0] + m[2][3] * p.m[3][0];
			o.m[2][1] = m[2][0] * p.m[0][1] + m[2][1] * p.m[1][1] + m[2][2] * p.m[2][1] + m[2][3] * p.m[3][1];
			o.m[2][2] = m[2][0] * p.m[0][2] + m[2][1] * p.m[1][2] + m[2][2] * p.m[2][2] + m[2][3] * p.m[3][2];
			o.m[2][3] = m[2][0] * p.m[0][3] + m[2][1] * p.m[1][3] + m[2][2] * p.m[2][3] + m[2][3] * p.m[3][3];

			o.m[3][0] = m[3][0] * p.m[0][0] + m[3][1] * p.m[1][0] + m[3][2] * p.m[2][0] + m[3][3] * p.m[3][0];
			o.m[3][1] = m[3][0] * p.m[0][1] + m[3][1] * p.m[1][1] + m[3][2] * p.m[2][1] + m[3][3] * p.m[3][1];
			o.m[3][2] = m[3][0] * p.m[0][2] + m[3][1] * p.m[1][2] + m[3][2] * p.m[2][2] + m[3][3] * p.m[3][2];
			o.m[3][3] = m[3][0] * p.m[0][3] + m[3][1] * p.m[1][3] + m[3][2] * p.m[2][3] + m[3][3] * p.m[3][3];

			return o;
		}

		Quaternion GetQuaternion();
		float determinant();
		const float minor(size_t x, size_t y);
		const Matrix adjoint();
		void invert();
		void transpose();

		Matrix& operator*= (const Matrix& p)
		{
			return *this = this->operator*(p);
		}

		operator float* ()
		{
			return (float*)this;
		}

		void Rotate_HPB(const double h, const double p, const double b);
		Matrix newRotate_HPB(const double h, const double p, double b);
		Vector3 getHPB();
		static Matrix identity();
	};
}