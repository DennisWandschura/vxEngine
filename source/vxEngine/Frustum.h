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

struct Frustum
{
	enum FrustumPlanes{ Left, Right, Top, Bottom, Near, Far };

	__m128 planes[6];

	Frustum() :planes(){}

	void update(const vx::mat4 &pvMatrix)
	{
		auto matrix = vx::MatrixTranspose(pvMatrix);

		auto tmpPlane = _mm_add_ps(matrix.c[0], matrix.c[3]);
		auto tmp = vx::length3(tmpPlane);
		planes[FrustumPlanes::Left] = _mm_div_ps(tmpPlane, tmp);

		tmpPlane = _mm_sub_ps(matrix.c[3], matrix.c[0]);
		tmp = vx::length3(tmpPlane);
		planes[FrustumPlanes::Right] = _mm_div_ps(tmpPlane, tmp);

		tmpPlane = _mm_add_ps(matrix.c[1], matrix.c[3]);
		tmp = vx::length3(tmpPlane);
		planes[FrustumPlanes::Bottom] = _mm_div_ps(tmpPlane, tmp);

		tmpPlane = _mm_sub_ps(matrix.c[3], matrix.c[1]);
		tmp = vx::length3(tmpPlane);
		planes[FrustumPlanes::Top] = _mm_div_ps(tmpPlane, tmp);

		tmpPlane = _mm_add_ps(matrix.c[2], matrix.c[3]);
		tmp = vx::length3(tmpPlane);
		planes[FrustumPlanes::Near] = _mm_div_ps(tmpPlane, tmp);

		tmpPlane = _mm_sub_ps(matrix.c[3], matrix.c[2]);
		tmp = vx::length3(tmpPlane);
		planes[FrustumPlanes::Far] = _mm_div_ps(tmpPlane, tmp);
	}
};