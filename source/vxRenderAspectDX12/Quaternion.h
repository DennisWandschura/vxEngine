#pragma once

/****************************************************************************

GPU Pro 5 : Quaternions revisited - sample code
All sample code written from scratch by Sergey Makeev specially for article.

Copyright (c) 2013, Sergey Makeev

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software.

2. If you use this software in a non-commercial product, an acknowledgment
in the product documentation would be appreciated but is not required.

3. If you use this software in a commercial product you requried to credit
the author.

4. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.

5. This notice may not be removed or altered from any source distribution.


Please let me know if you use the code in your products or have any questions or suggestions.

e-mail: sergeymakeev@inbox.ru
http://linkedin.com/in/sergeymakeev/

****************************************************************************/

#include "mat3.h"

struct Quaternion
{
	enum CheckSourceResultFlag
	{
		SOURCE_BASIS_LEFT_HANDED = 0x1,
		SOURCE_BASIS_HAVE_SCALE = 0x2
	};

	f32 x;
	f32 y;
	f32 z;
	f32 w;

	static u32 checkQuaternionSource(const vx::float3 &x, const vx::float3 &y, const vx::float3 &z)
	{
		u32 flag = 0;
		float basisCheck = vx::dot3(vx::cross(x, y), z);
		if (basisCheck < 0.0f)
		{
			flag |= SOURCE_BASIS_LEFT_HANDED;
		}
		if (abs(1.0f - abs(basisCheck)) >= 0.1f)
		{
			flag |= SOURCE_BASIS_HAVE_SCALE;
		}
		return flag;
	}

	static vx::float4 mat3ToQuaternion(const mat3 &m)
	{
		float tr = m.c[0].x + m.c[1].y + m.c[2].z;

		vx::float4 q(0, 0, 0, 1);
		if (tr > 0)
		{
			float S = sqrt(tr + 1.0f) * 2.0f; // S=4*qw 
			q.w = 0.25f * S;
			q.x = (m[2][1] - m[1][2]) / S;
			q.y = (m[0][2] - m[2][0]) / S;
			q.z = (m[1][0] - m[0][1]) / S;
		}
		else if ((m[0][0] > m[1][1])&(m[0][0] > m[2][2]))
		{
			float S = sqrt(1.0f + m[0][0] - m[1][1] - m[2][2]) * 2.f; // S=4*qx 
			q.w = (m[2][1] - m[1][2]) / S;
			q.x = 0.25f * S;
			q.y = (m[0][1] + m[1][0]) / S;
			q.z = (m[0][2] + m[2][0]) / S;
		}
		else if (m[1][1] > m[2][2])
		{
			float S = sqrt(1.0f + m[1][1] - m[0][0] - m[2][2]) * 2.f; // S=4*qy
			q.w = (m[0][2] - m[2][0]) / S;
			q.x = (m[0][1] + m[1][0]) / S;
			q.y = 0.25f * S;
			q.z = (m[1][2] + m[2][1]) / S;
		}
		else
		{
			float S = sqrt(1.0f + m[2][2] - m[0][0] - m[1][1]) * 2.f; // S=4*qz
			q.w = (m[1][0] - m[0][1]) / S;
			q.x = (m[0][2] + m[2][0]) / S;
			q.y = (m[1][2] + m[2][1]) / S;
			q.z = 0.25f * S;
		}

		return q;
	}

	Quaternion() :x(0), y(0), z(0), w(1) {}

	explicit Quaternion(const mat3 &rotationMatrix)
	{
		// "Source matrix invalid. Quaternions should be constructed from orthogonal, normalized, right-handed basis."
		VX_ASSERT(checkQuaternionSource(rotationMatrix[0], rotationMatrix[1], rotationMatrix[2]) == 0);

		// First compute squared magnitudes of quaternion components - at least one
		// will be greater than 0 since quaternion is unit magnitude
		float qs2 = 0.25f * (rotationMatrix[0][0] + rotationMatrix.c[1][1] + rotationMatrix.c[2][2] + 1.0f);
		float qx2 = qs2 - 0.5f * (rotationMatrix[1][1] + rotationMatrix.c[2][2]);
		float qy2 = qs2 - 0.5f * (rotationMatrix[2][2] + rotationMatrix.c[0][0]);
		float qz2 = qs2 - 0.5f * (rotationMatrix[0][0] + rotationMatrix.c[1][1]);

		// Find maximum magnitude component
		int n = (qs2 > qx2) ?
			((qs2 > qy2) ? ((qs2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
			((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));

		// Compute signed quaternion components using numerically stable method
		float tmp;
		switch (n)
		{
		case 0:
			w = sqrtf(qs2);
			tmp = 0.25f / w;
			x = (rotationMatrix.c[1][2] - rotationMatrix.c[2][1]) * tmp;
			y = (rotationMatrix.c[2][0] - rotationMatrix.c[0][2]) * tmp;
			z = (rotationMatrix.c[0][1] - rotationMatrix.c[1][0]) * tmp;
			break;
		case 1:
			x = sqrtf(qx2);
			tmp = 0.25f / x;
			w = (rotationMatrix.c[1][2] - rotationMatrix.c[2][1]) * tmp;
			y = (rotationMatrix.c[1][0] + rotationMatrix.c[0][1]) * tmp;
			z = (rotationMatrix.c[2][0] + rotationMatrix.c[0][2]) * tmp;
			break;
		case 2:
			y = sqrtf(qy2);
			tmp = 0.25f / y;
			w = (rotationMatrix.c[2][0] - rotationMatrix.c[0][2]) * tmp;
			z = (rotationMatrix.c[2][1] + rotationMatrix.c[1][2]) * tmp;
			x = (rotationMatrix.c[0][1] + rotationMatrix.c[1][0]) * tmp;
			break;
		case 3:
			z = sqrtf(qz2);
			tmp = 0.25f / z;
			w = (rotationMatrix.c[0][1] - rotationMatrix.c[1][0]) * tmp;
			x = (rotationMatrix.c[0][2] + rotationMatrix.c[2][0]) * tmp;
			y = (rotationMatrix.c[1][2] + rotationMatrix.c[2][1]) * tmp;
			break;
		}

		/*auto q = mat3ToQuaternion(rotationMatrix);
		x = q.x;
		y = q.y;
		z = q.z;
		w = q.w;*/

		//Make positive W
		if (w < 0)
		{
			x = -x;
			y = -y;
			z = -z;
			w = -w;
		}

		normalize();
	}

	void normalize()
	{
		f32 len = sqrtf(x*x + y*y + z*z + w*w);

		if (len < 0.000001f)
			return;

		float invLen = 1.0f / len;

		x *= invLen;
		y *= invLen;
		z *= invLen;
		w *= invLen;
	}
};

inline vx::float3 quaternionRotation(const vx::float3 &v, const vx::float4 &q)
{
	auto qxyz = vx::float3(q.x, q.y, q.z);
	return v + 2.0 * vx::cross(qxyz, vx::cross(qxyz, v) + q.w * v);
}

inline f32 dot(const Quaternion & lhs, const Quaternion & rhs)
{
	return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w);
}