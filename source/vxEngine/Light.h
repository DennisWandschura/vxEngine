#pragma once

#include <vxLib/math/Vector.h>

struct Light
{
	enum class Type : U32{Spotlight, Spherelight};

	Type m_lightType;
	vx::float3 m_position;
	vx::float3 m_direction;
	F32 m_falloff;
	F32 m_surfaceRadius;
	F32 m_lumen;
	F32 m_angle;
};