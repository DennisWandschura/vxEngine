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
#pragma once

class NavMesh;
class NavGraph;

#include <vxLib/math/Vector.h>
#include <vector>
#include "AABB.h"
#include <vxLib/memory.h>

struct InfluenceCell
{
	vx::float3 m_position;
	f32 m_influence;
	u16 m_offset;
	u16 m_count;
	f32 m_area;
	f32 m_priority;
};

class InfluenceMap
{
	struct AABB_SIMD
	{
		__m128 vmin, vmax;
	};

	std::unique_ptr<AABB_SIMD[]> m_cellBounds;
	std::unique_ptr<InfluenceCell[]> m_cells; // 8
	// influence cell data
	std::unique_ptr<u16[]> m_navNodeIndices;
	vx::float3 m_center; // 12
	vx::float3 m_invGridCellSize; // 12
	vx::float3 m_voxelHalfDim; // 12
	vx::ushort3 m_cellCount; // 6

	void createCells(const vx::uint3 &cellCount, u32 cellTotalCount, const vx::float3 &cellDim, const AABB &navBounds);
	void createCell(__m128 vBoundsMin, __m128 vCellSize, const vx::uint3 &cellPosition, u32 index);
	void createNodeIndicesAndSetCellData(const NavMesh &navMesh, u32 cellTotalCount);

public:
	void initialize(const NavMesh &navMesh, f32 cellSize, f32 cellHeight);

	void update(f32 dt);
	void updateActor(f32 dt, const vx::float3 &position);

	const InfluenceCell& getCell(u32 x, u32 y, u32 z) const;
	const InfluenceCell& getCell(u32 index) const;
	const InfluenceCell* getInfluenceCells() const;

	u32 getCellCount() const;

	s32 getClosestCellIndex_nocheck(const vx::float3 &position) const;
	u32 getClosestCellIndex(const vx::float3 &position) const;
	vx::int3 getClosestCellPosition(const vx::float3 &position) const;
	const u16* getNavNodeIndices() const;

	bool isEmpty(u32 cellIndex) const;
	bool contains(u32 cellIndex, const vx::float3 &position) const;

	void getCells(const vx::float3 &p, f32 minRadius, f32 maxRadius, u32 maxCount, InfluenceCell* pCells, u32* count) const;
};