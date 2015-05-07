/*
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

			v.x = node[0].as<u32>();
			v.y = node[1].as<u32>();

			return true;
		}
	};
}
#endif