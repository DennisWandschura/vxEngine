#pragma once

#include <vxLib/math/matrix.h>

struct Quaternion
{
	union
	{
		__m128 v;
		struct
		{
			F32 x, y, z, w;
		};
	};

	Quaternion(){}
	Quaternion(const __m128 &q) :v(q){}
	Quaternion(const Quaternion &q) :v(q.v){}

	static Quaternion VX_CALLCONV create(__m128 normalizedAxis, F32 angleInRad)
	{
		F32 halfAngle = angleInRad / 2.0f;

		F32 sinAngle, cosAngle;
		vx::scalarSinCos(&sinAngle, &cosAngle, halfAngle);

		__m128 angles = { sinAngle, sinAngle, sinAngle, cosAngle };

		normalizedAxis.f[3] = 1.0f;

		Quaternion q;
		q.v = _mm_mul_ps(normalizedAxis, angles);

		return q;
	}

	static Quaternion VX_CALLCONV create(const vx::mat4 &M)
	{
		static const __m128 XMPMMP = { +1.0f, -1.0f, -1.0f, +1.0f };
		static const __m128 XMMPMP = { -1.0f, +1.0f, -1.0f, +1.0f };
		static const __m128 XMMMPP = { -1.0f, -1.0f, +1.0f, +1.0f };

		auto r0 = M.c[0];  // (r00, r01, r02, 0)
		auto r1 = M.c[1];  // (r10, r11, r12, 0)
		auto r2 = M.c[2];  // (r20, r21, r22, 0)

		// (r00, r00, r00, r00)
		auto r00 = VX_PERMUTE_PS(r0, _MM_SHUFFLE(0, 0, 0, 0));
		// (r11, r11, r11, r11)
		auto r11 = VX_PERMUTE_PS(r1, _MM_SHUFFLE(1, 1, 1, 1));
		// (r22, r22, r22, r22)
		auto r22 = VX_PERMUTE_PS(r2, _MM_SHUFFLE(2, 2, 2, 2));

		// x^2 >= y^2 equivalent to r11 - r00 <= 0
		// (r11 - r00, r11 - r00, r11 - r00, r11 - r00)
		auto r11mr00 = _mm_sub_ps(r11, r00);
		auto x2gey2 = _mm_cmple_ps(r11mr00, vx::g_VXZero);

		// z^2 >= w^2 equivalent to r11 + r00 <= 0
		// (r11 + r00, r11 + r00, r11 + r00, r11 + r00)
		auto r11pr00 = _mm_add_ps(r11, r00);
		auto z2gew2 = _mm_cmple_ps(r11pr00, vx::g_VXZero);

		// x^2 + y^2 >= z^2 + w^2 equivalent to r22 <= 0
		auto x2py2gez2pw2 = _mm_cmple_ps(r22, vx::g_VXZero);

		// (+r00, -r00, -r00, +r00)
		auto t0 = _mm_mul_ps(XMPMMP, r00);

		// (-r11, +r11, -r11, +r11)
		auto t1 = _mm_mul_ps(XMMPMP, r11);

		// (-r22, -r22, +r22, +r22)
		auto t2 = _mm_mul_ps(XMMMPP, r22);

		// (4*x^2, 4*y^2, 4*z^2, 4*w^2)
		auto x2y2z2w2 = _mm_add_ps(t0, t1);
		x2y2z2w2 = _mm_add_ps(t2, x2y2z2w2);
		x2y2z2w2 = _mm_add_ps(x2y2z2w2, vx::g_VXOne);

		// (r01, r02, r12, r11)
		t0 = _mm_shuffle_ps(r0, r1, _MM_SHUFFLE(1, 2, 2, 1));
		// (r10, r10, r20, r21)
		t1 = _mm_shuffle_ps(r1, r2, _MM_SHUFFLE(1, 0, 0, 0));
		// (r10, r20, r21, r10)
		t1 = VX_PERMUTE_PS(t1, _MM_SHUFFLE(1, 3, 2, 0));
		// (4*x*y, 4*x*z, 4*y*z, unused)
		auto xyxzyz = _mm_add_ps(t0, t1);

		// (r21, r20, r10, r10)
		t0 = _mm_shuffle_ps(r2, r1, _MM_SHUFFLE(0, 0, 0, 1));
		// (r12, r12, r02, r01)
		t1 = _mm_shuffle_ps(r1, r0, _MM_SHUFFLE(1, 2, 2, 2));
		// (r12, r02, r01, r12)
		t1 = VX_PERMUTE_PS(t1, _MM_SHUFFLE(1, 3, 2, 0));
		// (4*x*w, 4*y*w, 4*z*w, unused)
		auto xwywzw = _mm_sub_ps(t0, t1);
		xwywzw = _mm_mul_ps(XMMPMP, xwywzw);

		// (4*x^2, 4*y^2, 4*x*y, unused)
		t0 = _mm_shuffle_ps(x2y2z2w2, xyxzyz, _MM_SHUFFLE(0, 0, 1, 0));
		// (4*z^2, 4*w^2, 4*z*w, unused)
		t1 = _mm_shuffle_ps(x2y2z2w2, xwywzw, _MM_SHUFFLE(0, 2, 3, 2));
		// (4*x*z, 4*y*z, 4*x*w, 4*y*w)
		t2 = _mm_shuffle_ps(xyxzyz, xwywzw, _MM_SHUFFLE(1, 0, 2, 1));

		// (4*x*x, 4*x*y, 4*x*z, 4*x*w)
		auto tensor0 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(2, 0, 2, 0));
		// (4*y*x, 4*y*y, 4*y*z, 4*y*w)
		auto tensor1 = _mm_shuffle_ps(t0, t2, _MM_SHUFFLE(3, 1, 1, 2));
		// (4*z*x, 4*z*y, 4*z*z, 4*z*w)
		auto tensor2 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(2, 0, 1, 0));
		// (4*w*x, 4*w*y, 4*w*z, 4*w*w)
		auto tensor3 = _mm_shuffle_ps(t2, t1, _MM_SHUFFLE(1, 2, 3, 2));

		// Select the row of the tensor-product matrix that has the largest
		// magnitude.
		t0 = _mm_and_ps(x2gey2, tensor0);
		t1 = _mm_andnot_ps(x2gey2, tensor1);
		t0 = _mm_or_ps(t0, t1);
		t1 = _mm_and_ps(z2gew2, tensor2);
		t2 = _mm_andnot_ps(z2gew2, tensor3);
		t1 = _mm_or_ps(t1, t2);
		t0 = _mm_and_ps(x2py2gez2pw2, t0);
		t1 = _mm_andnot_ps(x2py2gez2pw2, t1);
		t2 = _mm_or_ps(t0, t1);

		// Normalize the row.  No division by zero is possible because the
		// quaternion is unit-length (and the row is a nonzero multiple of
		// the quaternion).
		//t0 = DirectX::XMVector4Length(t2);
		//return _mm_div_ps(t2, t0);
		return vx::normalize(t2);
	}

	static Quaternion VX_CALLCONV normalize(const Quaternion &q)
	{
		return vx::normalize(q.v);
	}

	static vx::mat4 VX_CALLCONV convertToMatrix(const Quaternion &q)
	{
		F32 xsq = q.x * q.x;
		F32 ysq = q.y * q.y;
		F32 zsq = q.z * q.z;
		F32 wsq = q.w * q.w;

		F32 twoX = 2.0f * q.x;
		F32 twoY = 2.0f * q.y;
		F32 twoW = 2.0f * q.w;

		F32 xy = twoX * q.y;
		F32 xz = twoX * q.z;
		F32 yz = twoY * q.z;
		F32 wx = twoW * q.x;
		F32 wy = twoW * q.y;
		F32 wz = twoW * q.z;

		vx::mat4 result;
		result.c[0].f[0] = wsq + xsq - ysq - zsq;
		result.c[0].f[1] = xy + wz;
		result.c[0].f[2] = xz - wx;
		result.c[0].f[3] = 0.0f;

		result.c[1].f[0] = xy - wz;
		result.c[1].f[1] = wsq - xsq + ysq - zsq;
		result.c[1].f[2] = yz - wx;
		result.c[1].f[3] = 0.0f;

		result.c[2].f[0] = xz + wz;
		result.c[2].f[1] = yz - wx;
		result.c[2].f[2] = wsq - xsq - ysq + zsq;
		result.c[2].f[3] = 0.0f;

		result.c[3].f[0] = 0;
		result.c[3].f[1] = 0;
		result.c[3].f[2] = 0;
		result.c[3].f[3] = 1.0f;

		return result;
	}
};