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

#include <vxLib/math/matrix.h>
#include <vxEngineLib/Plane.h>

inline bool __vectorcall testFrustumPlane(__m128 c, __m128 r, const PlaneSIMD &p)
{
	static const __m128 zero = { 0, 0, 0, 0 };
	auto dist = vx::dot3(c, p.n);
	dist = _mm_sub_ps(dist, p.d);
	//dist = _mm_add_ps(dist, r);
	r.m128_f32[0] = -r.m128_f32[0];

	auto cmp = _mm_cmpge_ss(dist, r);
	auto mask = _mm_movemask_ps(cmp);

	return (mask & (1 << 0));
}

class Frustum
{
	struct PackedPlane
	{
		__m128 nd;
	};

	enum FrustumPlane { FrustumPlaneNear, FrustumPlaneLeft, FrustumPlaneRight, FrustumPlaneFar};

	PackedPlane m_planes[4];
	PackedPlane m_planeZ13;

public:
	Frustum();
	~Frustum();

	void update(const vx::mat4 &invPvMatrix);

	u32 __vectorcall testSpheres(__m128* cr, u32 count, u32* indices) const;

	void getPlaneZ13(PlaneSIMD* plane) const;
};