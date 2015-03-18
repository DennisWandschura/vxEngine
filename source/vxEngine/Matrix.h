#pragma once

#include <vxLib\math\Vector.h>

struct mat3
{
	vx::float3 c[3];

	mat3() :c(){}
	mat3(const vx::float3 &c0, const vx::float3 &c1, const vx::float3 &c2)
	{
		c[0] = c0;
		c[1] = c1;
		c[2] = c2;
	}

	F32 det()
	{
		F32 tmp0 = c[0].x * (c[1].y * c[2].z - c[1].z * c[2].y);
		F32 tmp1 = c[1].x * (c[0].y * c[2].z - c[0].z * c[2].y);
		F32 tmp2 = c[2].x * (c[0].y * c[1].z - c[0].z * c[1].y);

		return tmp0 + tmp1 + tmp2;
	}
};