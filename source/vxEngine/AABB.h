#pragma once

class Ray;

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

	AABB Union(const AABB &other) const;
	AABB Union(const vx::float3 &p) const;

	U32 maximumExtend() const;

	F32 surfaceArea() const;

	bool contains(const vx::float3 &p) const;
	bool contains(const __m128 &p) const;
	bool intersect(const Ray &ray, F32* hitt0, F32* hitt1) const;

	vx::float3& operator[](U32 i);

	const vx::float3& operator[](U32 i) const;
};

#pragma warning( pop ) 