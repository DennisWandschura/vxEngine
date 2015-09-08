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

struct mat3
{
	vx::float3 c[3];

	mat3() :c() {}
	mat3(const vx::float3 &c0, const vx::float3 &c1, const vx::float3 &c2)
	{
		c[0] = c0;
		c[1] = c1;
		c[2] = c2;
	}

	vx::float3& operator[](u32 i)
	{
		return c[i];
	}

	const vx::float3& operator[](u32 i) const
	{
		return c[i];
	}

	void makeRightHanded()
	{
		auto h = det();

		c[0].x *= h;
		c[1].x *= h;
		c[2].x *= h;
	}

	f32 det() const
	{
		vx::float3 r[3];
		r[0] = vx::float3(c[0].x, c[1].x, c[2].x);
		r[1] = vx::float3(c[0].y, c[1].y, c[2].y);
		r[2] = vx::float3(c[0].z, c[1].z, c[2].z);

		return vx::dot3(vx::cross(r[0], r[1]), r[2]);
	}

	friend vx::float3 operator*(const mat3 &m, const vx::float3 &v)
	{
		vx::float3 result;

		result.x = m.c[0].x * v.x + m.c[1].x * v.y + m.c[2].x * v.z;
		result.y = m.c[0].y * v.x + m.c[1].y * v.y + m.c[2].y * v.z;
		result.z = m.c[0].z * v.x + m.c[1].z * v.y + m.c[2].z * v.z;

		return result;
	}
};