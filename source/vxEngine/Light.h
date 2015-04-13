#pragma once

#include <vxLib/math/matrix.h>
#include "AABB.h"
#include <vector>

namespace YAML
{
	class Node;
}

struct Light
{
	vx::float3 m_position;
	vx::float3 m_direction;
	F32 m_falloff;
	F32 m_lumen;
	F32 m_angle;

	static std::vector<Light> loadFromYaml(const YAML::Node &n);
	static YAML::Node saveToYaml(const Light* lights, U32 count);

	void getTransformationMatrix(vx::mat4* m) const;
};