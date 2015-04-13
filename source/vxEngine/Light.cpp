﻿#include "Light.h"
#include "yamlHelper.h"

namespace YAML
{
	template<>
	struct convert < AABB >
	{
		static bool decode(const YAML::Node &node, AABB &data)
		{
			data.min = node["min"].as<vx::float3>();
			data.max = node["max"].as<vx::float3>();

			return true;
		}
	};

	template<>
	struct convert < Light >
	{
		static bool decode(const YAML::Node &node, Light &data)
		{
			data.m_position = node["position"].as<vx::float3>();
			data.m_direction = node["direction"].as<vx::float3>();
			data.m_falloff = node["falloff"].as<F32>();
			data.m_lumen = node["lumen"].as<F32>();
			data.m_angle = node["angle"].as<F32>();

			return true;
		}

		static Node encode(const Light &rhs)
		{
			Node node;
			node["position"] = rhs.m_position;
			node["direction"] = rhs.m_direction;
			node["falloff"] = rhs.m_falloff;
			node["lumen"] = rhs.m_lumen;
			node["angle"] = rhs.m_angle;

			return node;
		}
	};
}

std::vector<Light> Light::loadFromYaml(const YAML::Node &n)
{
	auto lights = n.as<std::vector<Light>>();
	return lights;
}

YAML::Node Light::saveToYaml(const Light* lights, U32 count)
{
	YAML::Node n;
	for (auto i = 0u; i < count; ++i)
	{
		n[i] = lights[i];
	}

	return n;
}

void Light::getTransformationMatrix(vx::mat4* m) const
{
	const __m128 x_axis = { 1, 0, 0, 0 };
	const __m128 y_axis = { 0, 1, 0, 0 };

	auto lightPos = vx::loadFloat(m_position);
	auto lightDir = vx::loadFloat(m_direction);

	auto upDir = vx::Vector3Cross(x_axis, lightDir);
	auto dot = vx::dot(upDir, upDir);
	if (dot == 0.0f)
	{
		upDir = vx::Vector3Cross(y_axis, lightDir);
	}

	auto projMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(m_angle), 1.0f, 0.1f, m_falloff);
	//auto projMatrix = vx::MatrixOrthographicRH(5, 6, 0.1f, light.m_falloff);
	auto viewMatrix = vx::MatrixLookToRH(lightPos, lightDir, upDir);

	*m = projMatrix * viewMatrix;

	// point light
	/*
	__m128 p = vx::loadFloat(light.m_position);
	*projMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(90.0), 1.0f, 0.1f, light.m_falloff);
	auto lightTranslationMatrix = vx::MatrixTranslation(-light.m_position.x, -light.m_position.y, -light.m_position.z);

	vx::mat4 viewMatrices[6];
	// X+
	vx::float4 up = { 0, -1, 0, 0 };
	vx::float4 dir = { 1, 0, 0, 0 };
	viewMatrices[0] = vx::MatrixLookToRH(p, vx::loadFloat(dir), vx::loadFloat(up));
	// X-
	up = { 0, -1, 0, 0 };
	dir = { -1, 0, 0, 0 };
	viewMatrices[1] = vx::MatrixLookToRH(p, vx::loadFloat(dir), vx::loadFloat(up));
	// Y+
	up = { 0, 0, 1, 0 };
	dir = vx::float4(0, 1, 0, 0);
	viewMatrices[2] = vx::MatrixLookToRH(p, vx::loadFloat(dir), vx::loadFloat(up));
	// Y-
	up = { 0, 0, -1, 0 };
	dir = vx::float4(0, -1, 0, 0);
	viewMatrices[3] = vx::MatrixLookToRH(p, vx::loadFloat(dir), vx::loadFloat(up));
	// Z+
	up = { 0, -1, 0, 0 };
	dir = vx::float4(0, 0, 1, 0);
	viewMatrices[4] = vx::MatrixLookToRH(p, vx::loadFloat(dir), vx::loadFloat(up));
	// Z-
	up = { 0, -1, 0, 0 };
	dir = vx::float4(0, 0, -1, 0);
	viewMatrices[5] = vx::MatrixLookToRH(p, vx::loadFloat(dir), vx::loadFloat(up));

	for (U32 i = 0; i < 6; ++i)
	{
	pvMatrices[i] = *projMatrix * viewMatrices[i];
	}
	*/
}