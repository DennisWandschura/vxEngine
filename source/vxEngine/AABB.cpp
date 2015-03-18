#include "AABB.h"
#include "Ray.h"
#include <algorithm>

AABB::AABB(const vx::float3 &p)
	:min(p),
	max(p)
{

}

AABB AABB::Union(const AABB &other) const
{
	AABB r;
	r.min = vx::min(min, other.min);
	r.max = vx::max(max, other.max);
	return r;
}

AABB AABB::Union(const vx::float3 &p) const
{
	AABB r;
	r.min = vx::min(min, p);
	r.max = vx::max(max, p);
	return r;
}

U32 AABB::maximumExtend() const
{
	auto d = max - min;

	U32 maxExtend = 0;
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

F32 AABB::surfaceArea() const
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

	auto c = (p.x >= min.x) & (p.x <= max.x) & (p.y >= min.y) & (p.y <= max.y) & (p.z >= min.z) & (p.z <= max.z);

	return c;
}

bool AABB::contains(const __m128 &p) const
{
	// only need to consider the first three values
	const int resultMask = 1 << 0 | 1 << 1 | 1 << 2;

	auto vmin = vx::loadFloat(&min);
	auto vmax = vx::loadFloat(&max);

	auto cmp1 = _mm_cmpge_ps(p, vmin);
	auto cmp2 = _mm_cmple_ps(p, vmax);

	auto m1 = _mm_movemask_ps(cmp1);
	auto m2 = _mm_movemask_ps(cmp2);

	auto m = (m1 & m2) & resultMask;

	return (m == resultMask);
}

bool AABB::intersect(const Ray &ray, F32* hitt0, F32* hitt1) const
{
	F32 t0 = ray.mint, t1 = ray.maxt;
	for (U32 i = 0; i < 3; ++i)
	{
		F32 invRayDir = 1.0f / ray.d[i];
		F32 tNear = (min[i] - ray.o[i]) * invRayDir;
		F32 tFar = (max[i] - ray.o[i]) * invRayDir;

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

vx::float3& AABB::operator[](U32 i)
{
	return v[i];
}

const vx::float3& AABB::operator[](U32 i) const
{
	return v[i];
}