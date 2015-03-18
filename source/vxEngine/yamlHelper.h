#ifndef __YAMLHELPER_H
#define __YAMLHELPER_H
#pragma once

#include <vxLib/math/Vector.h>
#include <yaml-cpp\yaml.h>

namespace YAML
{
	template<>
	struct convert<vx::float2>
	{
		static Node encode(const vx::float2 &rhs)
		{
			Node n;
			n[0] = rhs.x;
			n[1] = rhs.y;

			return n;
		}

		static bool decode(const Node &node, vx::float2 &v)
		{
			if (node.size() != 2 && !node.IsSequence())
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();

			return true;
		}
	};

	template<>
	struct convert<vx::float3>
	{
		static Node encode(const vx::float3 &rhs)
		{
			Node n;
			n[0] = rhs.x;
			n[1] = rhs.y;
			n[2] = rhs.z;

			return n;
		}

		static bool decode(const Node &node, vx::float3 &v)
		{
			if (node.size() != 3 && !node.IsSequence())
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();

			return true;
		}
	};

	template<>
	struct convert<vx::float4>
	{
		static Node encode(const vx::float4 &rhs)
		{
			Node n;
			n[0] = rhs.x;
			n[1] = rhs.y;
			n[2] = rhs.z;
			n[3] = rhs.w;

			return n;
		}

		static bool decode(const Node &node, vx::float4 &v)
		{
			if (node.size() != 4 && !node.IsSequence())
				return false;

			v.x = node[0].as<float>();
			v.y = node[1].as<float>();
			v.z = node[2].as<float>();
			v.w = node[3].as<float>();

			return true;
		}
	};

	template<>
	struct convert < vx::uint2 >
	{
		static Node encode(const vx::uint2 &rhs)
		{
			Node n;
			n[0] = rhs.x;
			n[1] = rhs.y;

			return n;
		}

		static bool decode(const Node &node, vx::uint2 &v)
		{
			if (node.size() != 2 && !node.IsSequence())
				return false;

			v.x = node[0].as<U32>();
			v.y = node[1].as<U32>();

			return true;
		}
	};
}
#endif