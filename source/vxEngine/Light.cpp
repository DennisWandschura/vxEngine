﻿/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "Light.h"
#include "GpuStructs.h"

/*namespace YAML
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
}*/

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
	//auto projMatrix = vx::MatrixOrthographicRH(8, 5, 0.01f, m_falloff);
	auto viewMatrix = vx::MatrixLookToRH(lightPos, lightDir, upDir);

	*m = projMatrix * viewMatrix;
}

void Light::getShadowTransform(PointLightShadowTransform* shadowTransform) const
{
	auto lightPos = vx::loadFloat(m_position);
	auto projectionMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(90.0f), 1.0f, 0.1f, m_falloff);

	vx::mat4 viewMatrices[6];
	// X+
	vx::float4 up = { 0, -1, 0, 0 };
	vx::float4 dir = { 1, 0, 0, 0 };
	viewMatrices[0] = vx::MatrixLookToRH(lightPos, vx::loadFloat(dir), vx::loadFloat(up));
	// X-
	up = { 0, -1, 0, 0 };
	dir = { -1, 0, 0, 0 };
	viewMatrices[1] = vx::MatrixLookToRH(lightPos, vx::loadFloat(dir), vx::loadFloat(up));
	// Y+
	up = { 0, 0, 1, 0 };
	dir = vx::float4(0, 1, 0, 0);
	viewMatrices[2] = vx::MatrixLookToRH(lightPos, vx::loadFloat(dir), vx::loadFloat(up));
	// Y-
	up = { 0, 0, -1, 0 };
	dir = vx::float4(0, -1, 0, 0);
	viewMatrices[3] = vx::MatrixLookToRH(lightPos, vx::loadFloat(dir), vx::loadFloat(up));
	// Z+
	up = { 0, -1, 0, 0 };
	dir = vx::float4(0, 0, 1, 0);
	viewMatrices[4] = vx::MatrixLookToRH(lightPos, vx::loadFloat(dir), vx::loadFloat(up));
	// Z-
	up = { 0, -1, 0, 0 };
	dir = vx::float4(0, 0, -1, 0);
	viewMatrices[5] = vx::MatrixLookToRH(lightPos, vx::loadFloat(dir), vx::loadFloat(up));

	shadowTransform->projectionMatrix = projectionMatrix;
	for (U32 i = 0; i < 6; ++i)
	{
		shadowTransform->viewMatrix[i] = viewMatrices[i];
		shadowTransform->pvMatrices[i] = projectionMatrix * viewMatrices[i];
	}
}