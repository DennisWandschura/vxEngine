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
#include <vxLib/StringID.h>
#include <vector>

enum class PlayerType : u32;

struct Spawn
{
	static u32 s_id;

	PlayerType type;
	vx::float3 position;
	// actor id, not used for human
	vx::StringID sid;
	u32 id;

	Spawn() 
		:type(),
		position(),
		sid(),
		id(s_id++)
	{}

	Spawn(const Spawn &rhs)
		:type(rhs.type),
		position(rhs.position),
		sid(rhs.sid),
		id(rhs.id)
	{
	}

	Spawn(Spawn &&rhs)
		:type(rhs.type),
		position(rhs.position),
		sid(rhs.sid),
		id(rhs.id)
	{
	}

	Spawn& operator=(const Spawn &rhs)
	{
		if (this != &rhs)
		{
			type = rhs.type;
			position = rhs.position;
			sid = rhs.sid;
			id = rhs.id;
		}
		return *this;
	}

	Spawn& operator=(Spawn &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(type, rhs.type);
			std::swap(position, rhs.position);
			std::swap(sid ,rhs.sid);
			std::swap(id,rhs.id);
		}
		return *this;
	}
};

struct SpawnFile
{
	PlayerType type;
	vx::float3 position;
	// actor id, not used for human
	char actor[16];

	SpawnFile()
	{
		actor[0] = '\0';
	}
};