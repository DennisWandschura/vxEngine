#pragma once

#include <vxLib/math/Vector.h>

struct Triangle
{
	vx::float3 m_points[3];

	vx::float3& operator[](U32 i)
	{
		return m_points[i];
	}

	const vx::float3& operator[](U32 i) const
	{
		return m_points[i];
	}

	F32 getArea() const;

	bool contains(const vx::float3 &point) const;
};

struct VX_ALIGN(8) TriangleA
{
	vx::float3 m_points[3];
	F32 padding;
};