#pragma once

#include <vxLib/math/Vector.h>

struct Triangle
{
	vx::float3 v[3];
};

struct VX_ALIGN(8) TriangleA
{
	vx::float3 v[3];
	F32 padding;
};