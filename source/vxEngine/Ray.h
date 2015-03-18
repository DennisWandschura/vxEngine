#pragma once

#include <vxLib/math/Vector.h>

class Ray
{
public:
	vx::float3 o;
	vx::float3 d;
	mutable F32 mint{ .0f }, maxt{ FLT_MAX };
	F32 time{ .0f };
	U32 depth{ 0 };

	Ray(){}

	vx::float3 operator()(F32 t) const
	{
		return o + d * t;
	}
};