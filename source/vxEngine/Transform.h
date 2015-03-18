#pragma once

#include <vxLib/math/Vector.h>

namespace vx
{
	struct Transform
	{
		vx::float3 m_translation{ 0, 0, 0 };
		vx::float3 m_rotation{ 0, 0, 0 };
		F32 m_scaling{ 1.0f };

		Transform() = default;

		Transform(const float3 &t, const float3 &r, const F32 s)
			:m_translation(t), m_rotation(r), m_scaling(s){}
	};

	struct VX_ALIGN(16) TransformGpu
	{
		vx::float3 translation;
		F32 scaling;
		vx::int2 packedQRotation;
	};
}