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

struct Plane
{
	vx::float3 n;
	f32 d;

	static Plane create(const vx::float3 &a, const vx::float3 &b, const vx::float3 &c)
	{
		auto tmp0 = b - a;
		auto tmp1 = c- a;

		Plane result;
		result.n = vx::normalize3(vx::cross(tmp0, tmp1));
		result.d = vx::dot3(result.n, a);

		return result;
	}
};

struct PlaneSIMD
{
	__m128 n;
	__m128 d;

	static PlaneSIMD __vectorcall create(__m128 a, __m128 b, __m128 c)
	{
		auto tmp0 = _mm_sub_ps(b, a);
		auto tmp1 = _mm_sub_ps(c, a);

		PlaneSIMD result;
		result.n = vx::normalize3(vx::cross3(tmp0, tmp1));
		result.d = vx::dot3(result.n, a);

		return result;
	}
};