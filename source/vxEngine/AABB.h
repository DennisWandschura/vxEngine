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