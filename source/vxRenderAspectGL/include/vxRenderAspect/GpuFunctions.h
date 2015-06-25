#pragma once
/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <vxLib/math/Vector.h>

struct GpuFunctions
{
	static vx::uint2 __vectorcall packQRotation(const __m128 qRotation)
	{
		const __m128 maxV = { 0xffff, 0xffff, 0xffff, 0xffff };

		auto qq = vx::fma(qRotation, vx::g_VXOneHalf, vx::g_VXOneHalf);
		qq = _mm_mul_ps(qq, maxV);

		vx::float4a tmp = qq;

		vx::uint2 packedQRotation;
		packedQRotation.x = (u32)tmp.x | (u32)tmp.y << 16;
		packedQRotation.y = (u32)tmp.z | (u32)tmp.w << 16;

		return packedQRotation;
	}

	static inline __m128 __vectorcall unpackQRotation(const vx::uint2 &rotation)
	{
		vx::uint4 tmp;
		tmp.x = rotation.x & 0xffff;
		tmp.y = (rotation.x >> 16) & 0xffff;
		tmp.z = rotation.y & 0xffff;
		tmp.w = (rotation.y >> 16) & 0xffff;

		const __m128 vmax = { 0xffff, 0xffff, 0xffff, 0xffff };

		__m128 qRotation;
		qRotation.m128_f32[0] = (float)tmp.x;
		qRotation.m128_f32[1] = (float)tmp.y;
		qRotation.m128_f32[2] = (float)tmp.z;
		qRotation.m128_f32[3] = (float)tmp.w;

		qRotation = _mm_div_ps(qRotation, vmax);
		qRotation = _mm_mul_ps(qRotation, vx::g_VXTwo);
		qRotation = _mm_sub_ps(qRotation, vx::g_VXOne);

		return qRotation;
	}
};