#pragma once

/* The MIT License (MIT)

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
SOFTWARE.*/

struct Entity;

#include <vxLib/math/Vector.h>
#include <vxEngineLib/AABB.h>
#include <vector>

struct QuadTreeData
{
	Entity* entity;
	vx::float3 position;
	vx::float3 velocity;
};

class QuadTree
{
	struct Cell
	{
		u32 m_dataOffset;
		u32 m_count;
	};

	AABB m_bounds;
	std::vector<Cell> m_cells;
	vx::float2a m_invGridDim;
	vx::uint2a m_dim;
	std::vector<QuadTreeData> m_data;
	u32 m_capacity;
	u32 m_size;

	void getDataLinear(const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* data) const;
	void getDataLinear(const Entity* entity, const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* data) const;

public:
	QuadTree();
	~QuadTree();

	void initialize(const AABB &bounds, const vx::uint2 &dim, u32 capacity);

	void clear();
	void insert(const QuadTreeData* data, u32 count);

	void getData(const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* data) const;
	void getData(const Entity* entity, const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* data) const;
};
