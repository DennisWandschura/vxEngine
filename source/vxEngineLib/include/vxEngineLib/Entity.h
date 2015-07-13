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

#include <vxLib/types.h>
#include <vxLib/math/Vector.h>
#include <vxLib/StringID.h>
#include <vxEngineLib/Component.h>

const u32 g_maxEntities = 128u;

struct ComponentEntry
{
	u8 type;
	u8 index;
};

struct Entity
{
	vx::float3 position;
	//vx::float2 orientation;
	vx::float4 qRotation;

	u8 size;
	ComponentEntry components[8];

	Entity() :position(), qRotation(), size(0), components() {}

	template<typename T>
	bool getComponentIndex(u8* index) const
	{
		bool result = false;

		auto type = T::s_type;
		for (u8 i = 0;i < size; ++i)
		{
			if (type == components[i].type)
			{
				*index = components[i].index;
				result = true;
				break;
			}
		}

		return result;
	}

	template<typename T>
	bool addComponent(const Component::Type<T> &component, u8 index)
	{
		if (size == 8)
			return false;

		components[size].type = Component::Type<T>::s_type;
		components[size].index = index;

		++size;

		return true;
	}
};