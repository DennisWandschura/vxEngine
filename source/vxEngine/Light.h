#pragma once

#include <vxLib/math/Vector.h>

struct Light
{
	vx::float3 m_position;
	F32 m_falloff;
	F32 m_surfaceRadius;
	F32 m_lumen;
};

struct AABBLight
{
	vx::float3 m_position;
	F32 m_lumen;
	vx::float3 m_halfSize;
	F32 m_falloff;
	U32 m_type;
	U32 padding;
};