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

#include "QuadTree.h"
#include <vxLib/memory.h>
#include <vxEngineLib/Entity.h>

QuadTree::QuadTree()
	:m_bounds(),
	m_cells(),
	m_dim(0, 0),
	m_data(),
	m_capacity(0),
	m_size(0)
{

}

QuadTree::~QuadTree()
{
	/*if (m_cells != nullptr)
	{
		delete[](m_cells);
		m_cells = nullptr;
	}*/

	/*if (m_data != nullptr)
	{
		delete[](m_data);
		m_data = nullptr;
	}*/
}

void QuadTree::initialize(const AABB &bounds, const vx::uint2 &dim, u32 capacity)
{
	m_bounds = bounds;
	auto cellCount = dim.x*dim.y;
	//m_cells = new Cell[cellCount];
	m_cells.reserve(cellCount);
	for (u32 i = 0;i < cellCount; ++i)
	{
		Cell cell;
		cell.m_count = 0;
		cell.m_dataOffset = 0;

		m_cells.push_back(cell);
	}

	vx::float2a halfExtend;
	halfExtend.x = (bounds.max.x - bounds.min.x) * 0.5f;
	halfExtend.y = (bounds.max.z - bounds.min.z) * 0.5f;

	vx::float2a halfDim = vx::float2a(dim) / 2;

	m_invGridDim = 1.0 / (halfExtend / halfDim);

	//m_data = new u32[capacity];
	m_data.reserve(capacity);
	for (u32 i = 0;i < capacity; ++i)
	{
		m_data.push_back(QuadTreeData());
	}

	m_dim = dim;
	m_capacity = capacity;
}

void QuadTree::clear()
{
	auto cellCount = m_dim.x * m_dim.y;
	for (u32 i = 0;i < cellCount; ++i)
	{
		m_cells[i].m_count = 0;
		m_cells[i].m_dataOffset = 0;
	}
}

void QuadTree::insert(const QuadTreeData* data, u32 count)
{
	if (count == 0)
		return;

	VX_ASSERT(m_capacity >= count);

	auto center = m_bounds.max + m_bounds.min;
	u32 dimX = m_dim.x;
	auto halfDim = m_dim / 2;
	auto invGridDim = vx::float3(m_invGridDim.x, 1.0f, m_invGridDim.y);

	struct Entry
	{
		u32 cellIndex;
		u32 dataIndex;
	};

	auto maxDim = m_dim - 1;

	auto cellIndices = vx::make_unique<Entry[]>(count);
	u32 insertedCount = 0;
	for (u32 i = 0; i < count; ++i)
	{
		auto position = data[i].position;

		if (m_bounds.contains(position))
		{
			position = (position - center) * invGridDim;

			vx::uint2a cell;
			cell.x = position.x + halfDim.x;
			cell.y = position.z + halfDim.y;

			cell = vx::max(cell, vx::uint2a(0, 0));
			cell = vx::min(cell, maxDim);

			auto cellIndex = cell.y * dimX + cell.x;
			cellIndices[insertedCount].cellIndex = cellIndex;
			cellIndices[insertedCount].dataIndex = i;
			++insertedCount;
		}
	}

	if (insertedCount == 0)
		return;

	std::sort(cellIndices.get(), cellIndices.get() + insertedCount, [](const Entry &lhs, const Entry &rhs)
	{
		return lhs.cellIndex < rhs.cellIndex;
	});

	u32 currentCellIndex = cellIndices[0].cellIndex;
	u32 currentCount = 1;
	u32 currentDataOffset = 0;
	u32 dataOffset = 0;

	auto currentItemIndex = cellIndices[0].dataIndex;
	m_data[dataOffset] = data[currentItemIndex];
	++dataOffset;

	for (u32 i = 1; i < insertedCount; ++i)
	{
		u32 cellIndex = cellIndices[i].cellIndex;

		if (currentCellIndex != cellIndex)
		{
			auto &cell = m_cells[currentCellIndex];
			cell.m_count = currentCount;
			cell.m_dataOffset = currentDataOffset;

			currentDataOffset += currentCount;
			currentCellIndex = cellIndex;
			currentCount = 0;
		}

		auto currentItemIndex = cellIndices[i].dataIndex;
		m_data[dataOffset] = data[currentItemIndex];

		++dataOffset;
		++currentCount;
	}

	auto &cell = m_cells[currentCellIndex];
	cell.m_count = currentCount;
	cell.m_dataOffset = currentDataOffset;

	m_size = insertedCount;
}

void QuadTree::getDataLinear(const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* outData) const
{
	auto size = m_size;
	auto data = m_data.data();

	u32 index = 0;
	for (u32 i = 0; i < size; ++i)
	{
		auto &it = data[i];

		auto distance = vx::distance3(it.position, position);
		if (distance <= radius)
		{
			outData[index] = it;
			++index;
		}

		if (maxCount != 0 && index == maxCount)
			break;
	}

	*count = index;
}

void QuadTree::getDataLinear(const EntityActor* entity, const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* outData) const
{
	auto size = m_size;
	auto data = m_data.data();

	u32 index = 0;
	for (u32 i = 0; i < size; ++i)
	{
		auto &it = data[i];

		auto distance = vx::distance3(it.position, position);
		if (entity != it.entity && 
			distance <= radius)
		{
			outData[index] = it;
			++index;
		}

		if (maxCount != 0 && index == maxCount)
			break;
	}

	*count = index;
}

void QuadTree::getData(const EntityActor* entity, const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* data) const
{
	getDataLinear(entity, position, radius, maxCount, count, data);
}

void QuadTree::getData(const vx::float3 &position, f32 radius, u32 maxCount, u32* count, QuadTreeData* data) const
{
	if (m_size <= 10)
	{
		getDataLinear(position, radius, maxCount, count, data);
	}
	/*auto invGridDim = vx::float3(m_invGridDim.x, 1.0f, m_invGridDim.y);
	auto center = m_bounds.max + m_bounds.min;

	//auto dim = m_dim;
	u32 dimX = m_dim.x;
	auto halfDim = m_dim / 2;

	auto vmin = position - vx::float3(radius);
	auto vmax = position + vx::float3(radius);

	vmin = (vmin - center) * invGridDim;
	vmax = (vmax - center) * invGridDim;

	vx::uint2a cellMin;
	cellMin.x = vmin.x + halfDim.x;
	cellMin.y = vmin.z + halfDim.y;

	auto maxDim = m_dim - 1;

	cellMin = vx::max(cellMin, vx::uint2a(0, 0));
	cellMin = vx::min(cellMin, maxDim);

	vx::uint2a cellMax;
	cellMax.x = vmax.x + halfDim.x;
	cellMax.y = vmax.z + halfDim.y;
	cellMax = vx::min(cellMax, maxDim);

	vx::uint2a cellCount = cellMax - cellMin;
	u32 outCount = 0;
	u32 outOffset = 0;

	u32 maxIndex = m_dim.x * m_dim.y;

	auto currentCell = cellMin;
	for (u32 y = 0; y <= cellCount.y; ++y)
	{
		for (u32 x = 0; x <= cellCount.x; ++x)
		{
			auto currentIndex = currentCell.x + currentCell.y * dimX;

			if (currentIndex >= maxIndex)
			{
				puts("");
			}

			auto &cell = m_cells[currentIndex];
			if (cell.m_count != 0)
			{
				auto currentCellCount = cell.m_count;
				auto offset = cell.m_dataOffset;
				for (u32 i = 0;i < currentCellCount; ++i)
				{
					data[outOffset] = m_data[offset];

					++offset;
					++outOffset;
				}

				outCount += currentCellCount;
			}

			++currentCell.x;
		}
		currentCell.x = cellMin.x;
		++currentCell.y;
	}

	*count = outCount;

	/*position = (position - center) * invGridDim;

	vx::uint2 cell;
	cell.x = position.x + dimX;
	cell.y = position.z + dimY;

	auto cellIndex = cell.y * dimX + cell.x;*/
}