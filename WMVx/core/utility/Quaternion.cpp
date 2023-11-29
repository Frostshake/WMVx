#include "../../stdafx.h"
#include "Quaternion.h"
#include "Matrix.h"

namespace core {

	const Quaternion Quaternion::slerp(const float r, const Quaternion& v1, const Quaternion& v2)
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

	const Quaternion Quaternion::lerp(const float r, const Quaternion& v1, const Quaternion& v2)
	{
		return v1 * (1.0f - r) + v2 * r;
	}

	Vector3 Quaternion::getHPB() const
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

	Matrix Quaternion::toMat() const
	{
		Matrix result;

		float fTx = ((float)2.0) * y;
		float fTy = ((float)2.0) * z;
		float fTz = ((float)2.0) * w;
		float fTwx = fTx * x;
		float fTwy = fTy * x;
		float fTwz = fTz * x;
		float fTxx = fTx * y;
		float fTxy = fTy * y;
		float fTxz = fTz * y;
		float fTyy = fTy * z;
		float fTyz = fTz * z;
		float fTzz = fTz * w;

		result.m[0][0] = (float)1.0 - (fTyy + fTzz);
		result.m[0][1] = fTxy - fTwz;
		result.m[0][2] = fTxz + fTwy;
		result.m[1][0] = fTxy + fTwz;
		result.m[1][1] = (float)1.0 - (fTxx + fTzz);
		result.m[1][2] = fTyz - fTwx;
		result.m[2][0] = fTxz - fTwy;
		result.m[2][1] = fTyz + fTwx;
		result.m[2][2] = (float)1.0 - (fTxx + fTyy);

		return result;
	}

	Vector3 Quaternion::toEulerXYZ() const
	{
		Vector3 result;

		Matrix mat = toMat();

		if (mat.m[0][2] < (float)1.0)
		{
			if (mat.m[0][2] > -(float)1.0)
			{
				// y_angle = asin(r02)
						// x_angle = atan2(-r12,r22)
						// z_angle = atan2(-r01,r00)
				result.y = (float)asin((double)mat.m[0][2]);
				result.x = atan2(-mat.m[1][2], mat.m[2][2]);
				result.z = atan2(-mat.m[0][1], mat.m[0][0]);
			}
			else
			{
				// y_angle = -pi/2
				// z_angle - x_angle = atan2(r10,r11)
				// WARNING.  Solution is not unique.  Choosing z_angle = 0.
				result.y = -HALFPIf;
				result.x = -atan2(mat.m[1][0], mat.m[1][1]);
				result.z = (float)0.0f;
			}
		}
		else
		{
			// y_angle = +pi/2
			// z_angle + x_angle = atan2(r10,r11)
			// WARNING.  Solutions is not unique.  Choosing z_angle = 0.
			result.y = HALFPIf;
			result.x = atan2(mat.m[1][0], mat.m[1][1]);
			result.z = (float)0.0f;
		}

		return result;
	}

};