#pragma once
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

#include <vxLib/math/Vector.h>

namespace Parser
{
	template<typename T>
	struct Converter;

	template<>
	struct Converter < vx::int2 >
	{
		static bool decode(const Node &node, vx::int2* data)
		{
			if (!node.isArray() && node.size() != 2)
				return false;

			node.as(0u, &data->v[0]);
			node.as(1u, &data->v[1]);

			return true;
		}
	};

	template<>
	struct Converter < vx::uint2 >
	{
		static bool decode(const Node &node, vx::uint2* data)
		{
			if (!node.isArray() && node.size() != 2)
				return false;

			node.as(0u, &data->v[0]);
			node.as(1u, &data->v[1]);

			return true;
		}
	};
}