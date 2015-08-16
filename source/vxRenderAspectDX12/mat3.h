#pragma once

#include <vxLib/math/Vector.h>

struct mat3
{
	vx::float3 c[3];

	mat3() :c() {}
	mat3(const vx::float3 &c0, const vx::float3 &c1, const vx::float3 &c2)
	{
		c[0] = c0;
		c[1] = c1;
		c[2] = c2;
	}

	vx::float3& operator[](u32 i)
	{
		return c[i];
	}

	const vx::float3& operator[](u32 i) const
	{
		return c[i];
	}

	void makeRightHanded()
	{
		auto h = det();

		c[0].x *= h;
		c[1].x *= h;
		c[2].x *= h;
	}

	f32 det() const
	{
		vx::float3 r[3];
		r[0] = vx::float3(c[0].x, c[1].x, c[2].x);
		r[1] = vx::float3(c[0].y, c[1].y, c[2].y);
		r[2] = vx::float3(c[0].z, c[1].z, c[2].z);

		return vx::dot3(vx::cross(r[0], r[1]), r[2]);
	}

	friend vx::float3 operator*(const mat3 &m, const vx::float3 &v)
	{
		vx::float3 result;

		result.x = m.c[0].x * v.x + m.c[1].x * v.y + m.c[2].x * v.z;
		result.y = m.c[0].y * v.x + m.c[1].y * v.y + m.c[2].y * v.z;
		result.z = m.c[0].z * v.x + m.c[1].z * v.y + m.c[2].z * v.z;

		return result;
	}
};