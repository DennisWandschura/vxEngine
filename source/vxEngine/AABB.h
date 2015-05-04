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
#pragma once

class Ray;
struct Triangle;
struct Plane;

#include <vxLib/math/Vector.h>
#include <float.h>

#pragma warning( push )
#pragma warning( disable : 337 )

struct AABB
{
	union
	{
		vx::float3 v[2];
		struct
		{
			vx::float3 min{ FLT_MAX, FLT_MAX, FLT_MAX };
			vx::float3 max{ -FLT_MAX, -FLT_MAX, -FLT_MAX };
		};
	};

	AABB() = default;
	AABB(const vx::float3 &p);

	static AABB merge(const AABB &a, const AABB &b);
	static AABB merge(const AABB &a, const vx::float3 &p);

	U32 maximumExtend() const;

	F32 surfaceArea() const;

	bool contains(const vx::float3 &p) const;
	bool contains(const __m128 &p) const;
	bool intersects(const Ray &ray, F32* hitt0, F32* hitt1) const;
	bool intersects(const Triangle &triangle);
	bool intersects(const Plane &plane);

	vx::float3& operator[](U32 i);

	const vx::float3& operator[](U32 i) const;
};

#pragma warning( pop ) 