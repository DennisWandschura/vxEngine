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
#include "AABB.h"
#include "Ray.h"
#include <algorithm>
#include "Triangle.h"
#include "Plane.h"

AABB::AABB()
	:min(FLT_MAX, FLT_MAX, FLT_MAX),
	max(-FLT_MAX, -FLT_MAX, -FLT_MAX)
{
}

AABB::AABB(const vx::float3 &p)
	:min(p),
	max(p)
{

}

AABB AABB::merge(const AABB &a, const AABB &b)
{
	AABB r;
	r.min = vx::min(a.min, b.min);
	r.max = vx::max(a.max, b.max);
	return r;
}

AABB AABB::merge(const AABB &a, const vx::float3 &p)
{
	AABB r;
	r.min = vx::min(a.min, p);
	r.max = vx::max(a.max, p);
	return r;
}

u32 AABB::maximumExtend() const
{
	auto d = max - min;

	u32 maxExtend = 0;
	if (d.x > d.y && d.x > d.z)
	{
		maxExtend = 0;
	}
	else if (d.y > d.z)
	{
		maxExtend = 1;
	}
	else
	{
		maxExtend = 2;
	}

	return maxExtend;
}

f32 AABB::surfaceArea() const
{
	auto d = max - min;
	return 2.0f * (d.x * d.y + d.x * d.z + d.y*d.z);
}

bool AABB::contains(const vx::float3 &p) const
{
	/*if (p.x >= min.x && p.x <= max.x &&
		p.y >= min.y && p.y <= max.y &&
		p.z >= min.z && p.z <= max.z)
	{
		return true;
	}*/

	/*auto px = _mm_load_ss(&p.x);
	auto py = _mm_load_ss(&p.y);
	auto pz = _mm_load_ss(&p.z);

	auto minx = _mm_load_ss(&min.x);
	auto miny = _mm_load_ss(&min.y);
	auto minz = _mm_load_ss(&min.z);

	minx = _mm_cmpge_ss(px, minx);
	miny = _mm_cmpge_ss(py, miny);
	minz = _mm_cmpge_ss(pz, minz);

	//minx = _mm_cmpge_ss(px, minx);
	//miny = _mm_cmpge_ss(py, miny);
	//minz = _mm_cmpge_ss(pz, minz);

	auto l = _mm_movemask_ps(minx);*/

	// 4 * 6 = 24

	if (p.x >= min.x && p.x <= max.x &&
		p.y >= min.y && p.y <= max.y &&
		p.z >= min.z && p.z <= max.z)
	{
		return true;
	}

	return false;
}

bool AABB::contains(const __m128 &p) const
{
	// only need to consider the first three values
	const int resultMask = 1 << 0 | 1 << 1 | 1 << 2;

	auto vmin = vx::loadFloat(min);
	auto vmax = vx::loadFloat(max);

	auto cmp1 = _mm_cmpge_ps(p, vmin);
	auto cmp2 = _mm_cmple_ps(p, vmax);

	auto m1 = _mm_movemask_ps(cmp1);
	auto m2 = _mm_movemask_ps(cmp2);

	auto m = (m1 & m2) & resultMask;

	return (m == resultMask);
}

bool AABB::intersects(const Ray &ray, f32* hitt0, f32* hitt1) const
{
	f32 t0 = ray.mint, t1 = ray.maxt;

	/*vx::float3 invRayDir = 1.0f / ray.d;

	auto tNear = (min - ray.o) * invRayDir;
	auto tFar = (max - ray.o) * invRayDir;

	auto tmin =*/

	for (u32 i = 0; i < 3; ++i)
	{
		f32 invRayDir = 1.0f / ray.d[i];
		f32 tNear = (min[i] - ray.o[i]) * invRayDir;
		f32 tFar = (max[i] - ray.o[i]) * invRayDir;

		if (tNear > tFar)
			std::swap(tNear, tFar);

		t0 = tNear > t0 ? tNear : t0;
		t1 = tFar < t1 ? tFar : t1;

		if (t0 > t1)
			return false;
	}

	if (hitt0) *hitt0 = t0;
	if (hitt1) *hitt1 = t1;
	return true;
}

bool AABB::intersects(const Triangle &triangle)
{
	auto c = (min + max) * 0.5f;
	auto e = (max - min) * 0.5f;

	auto v0 = triangle[0] - c;
	auto v1 = triangle[1] - c;
	auto v2 = triangle[2] - c;

	auto f0 = v1 - v0;
	auto f1 = v2 - v1;
	auto f2 = v0 - v2;

	// a00
	auto f = vx::float3(0, -f0.z, f0.y);
	auto p0 = vx::dot(v0, f);
	auto p1 = vx::dot(v1, f);
	auto p2 = vx::dot(v2, f);
	auto r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a01
	f = vx::float3(0, -f1.z, f1.y);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a02
	f = vx::float3(0, -f2.z, f2.y);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a10
	f = vx::float3(f0.z, 0, -f0.x);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a11
	f = vx::float3(f1.z, 0, -f1.x);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a12
	f = vx::float3(f2.z, 0, -f2.x);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a20
	f = vx::float3(-f0.y, f0.x, 0);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a21
	f = vx::float3(-f1.y, f1.x, 0);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	// a22
	f = vx::float3(-f2.y, f2.x, 0);
	p0 = vx::dot(v0, f);
	p1 = vx::dot(v1, f);
	p2 = vx::dot(v2, f);
	r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
	if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

	auto max3 = [](f32 a, f32 b, f32 c)
	{
		return std::max(a, std::max(b, c));
	};

	auto min3 = [](f32 a, f32 b, f32 c)
	{
		return std::min(a, std::min(b, c));
	};

	if (max3(v0.x, v1.x, v2.x) < -e.x || min3(v0.x, v1.x, v2.x) > e.x) return false;
	if (max3(v0.y, v1.y, v2.y) < -e.y || min3(v0.y, v1.y, v2.y) > e.y) return false;
	if (max3(v0.z, v1.z, v2.z) < -e.z || min3(v0.z, v1.z, v2.z) > e.z) return false;

	Plane p;
	p.n = vx::cross(f0, f1);
	p.d = vx::dot(p.n, v0);

	return intersects(p);
}

bool AABB::intersects(const Plane &plane)
{
	auto c = (max + min) * 0.5f;
	auto e = max - c;

	auto r = e.x * abs(plane.n.x) + e.y * abs(plane.n.y) + e.z * abs(plane.n.z);
	auto s = vx::dot(plane.n, c) - plane.d;

	return fabsf(s) <= r;
}

vx::float3& AABB::operator[](u32 i)
{
	return v[i];
}

const vx::float3& AABB::operator[](u32 i) const
{
	return v[i];
}