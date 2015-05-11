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
#include "InfluenceMap.h"
#include "NavMesh.h"
#include <random>
#include <algorithm>
#include "NavGraph.h"
#include "NavNode.h"
#include "NavMeshTriangle.h"

namespace
{
	struct Triangle_SIMD
	{
		__m128 v[3];
	};

	struct AABB_POINT
	{
		static bool VX_CALLCONV contains(const __m128 vmin, const __m128 vmax, const __m128 p)
		{
			// only need to consider the first three values
			const int resultMask = 1 << 0 | 1 << 1 | 1 << 2;

			auto cmp1 = _mm_cmpge_ps(p, vmin);
			auto cmp2 = _mm_cmple_ps(p, vmax);

			auto m1 = _mm_movemask_ps(cmp1);
			auto m2 = _mm_movemask_ps(cmp2);

			auto m = (m1 & m2) & resultMask;

			return (m == resultMask);
		}
	};

	/*bool VX_CALLCONV intersects(const __m128 vmin, const __m128 vmax, const Triangle_SIMD &triangle)
	{
		auto c = _mm_mul_ps(_mm_add_ps(vmin, vmax), vx::g_VXOneHalf);
		auto e = _mm_mul_ps(_mm_sub_ps(vmax, vmin), vx::g_VXOneHalf);

		auto v0 = _mm_sub_ps(triangle.v[0], c);
		auto v1 = _mm_sub_ps(triangle.v[1], c);
		auto v2 = _mm_sub_ps(triangle.v[2], c);

		auto f0 = _mm_sub_ps(v1, v0);
		auto f1 = _mm_sub_ps(v2, v1);
		auto f2 = _mm_sub_ps(v0, v2);

		const __m128 mask0 = {0, -1.0f, 1.0f, 0.0f};

		__m128 f = VX_PERMUTE_PS(f0, _MM_SHUFFLE(0, 1, 2, 0));
		// a00
		f = _mm_mul_ps(f, mask0);
		auto absF = vx::detail::abs(f);

		auto p0 = vx::Vector3Dot(v0, f);
		auto p1 = vx::Vector3Dot(v1, f);
		auto p2 = vx::Vector3Dot(v2, f);

		auto tmp = vx::VectorNegate(_mm_max_ps(p0, p2));
		auto tmp1 = _mm_min_ps(p0, p2);
		auto r0 = vx::Vector3Dot(e, absF);

		auto cmp = _mm_max_ps(tmp, tmp1);

		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a01
		f = VX_PERMUTE_PS(f1, _MM_SHUFFLE(0, 1, 2, 0));
		f = _mm_mul_ps(f, mask0);
		absF = vx::detail::abs(f);
		p0 = vx::Vector3Dot(v0, f);
		p1 = vx::Vector3Dot(v1, f);
		p2 = vx::Vector3Dot(v2, f);

		tmp = vx::VectorNegate(_mm_max_ps(p0, p2));
		tmp1 = _mm_min_ps(p0, p2);
		auto r1 = vx::Vector3Dot(e, absF);

		cmp = _mm_shuffle_ps(cmp, _mm_max_ps(tmp, tmp1), _MM_SHUFFLE(1, 0, 1, 0));

		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a02
		f = VX_PERMUTE_PS(f2, _MM_SHUFFLE(0, 1, 2, 0));
		f = _mm_mul_ps(f, mask0);
		absF = vx::detail::abs(f);
		p0 = vx::Vector3Dot(v0, f);
		p1 = vx::Vector3Dot(v1, f);
		p2 = vx::Vector3Dot(v2, f);
		tmp = vx::VectorNegate(_mm_max_ps(p0, p2));
		tmp1 = _mm_min_ps(p0, p2);

		auto r2 = vx::Vector3Dot(e, absF);
		cmp = _mm_shuffle_ps(cmp, _mm_max_ps(tmp, tmp1), _MM_SHUFFLE(1, 0, 2, 0));

		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a10
		f = vx::float3(f0.z, 0, -f0.x);
		p0 = vx::dot(v0, f);
		p1 = vx::dot(v1, f);
		p2 = vx::dot(v2, f);
		r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a11
		f = vx::float3(f1.z, 0, -f1.x);
		p0 = vx::dot(v0, f);
		p1 = vx::dot(v1, f);
		p2 = vx::dot(v2, f);
		r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a12
		f = vx::float3(f2.z, 0, -f2.x);
		p0 = vx::dot(v0, f);
		p1 = vx::dot(v1, f);
		p2 = vx::dot(v2, f);
		r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a20
		f = vx::float3(-f0.y, f0.x, 0);
		p0 = vx::dot(v0, f);
		p1 = vx::dot(v1, f);
		p2 = vx::dot(v2, f);
		r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a21
		f = vx::float3(-f1.y, f1.x, 0);
		p0 = vx::dot(v0, f);
		p1 = vx::dot(v1, f);
		p2 = vx::dot(v2, f);
		r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		// a22
		f = vx::float3(-f2.y, f2.x, 0);
		p0 = vx::dot(v0, f);
		p1 = vx::dot(v1, f);
		p2 = vx::dot(v2, f);
		r = e.x * fabsf(f.x) + e.y * fabsf(f.y) + e.z * fabsf(f.z);
		if (fmaxf(-fmaxf(p0, p2), fminf(p0, p2)) > r) return false;

		auto max3 = [](f32 a, f32 b, f32 c)
		{
			return std::max(a, std::max(b, c));
		};

		auto min3 = [](f32 a, f32 b, f32 c)
		{
			return std::min(a, std::min(b, c));
		};

		if (max3(v0.x, v1.x, v2.x) < -e.x || min3(v0.x, v1.x, v2.x) > e.x) return false;
		if (max3(v0.y, v1.y, v2.y) < -e.y || min3(v0.y, v1.y, v2.y) > e.y) return false;
		if (max3(v0.z, v1.z, v2.z) < -e.z || min3(v0.z, v1.z, v2.z) > e.z) return false;

		Plane p;
		p.n = vx::cross(f0, f1);
		p.d = vx::dot(p.n, v0);

		auto c = (max + min) * 0.5f;
		auto e = max - c;

		auto r = e.x * abs(p.n.x) + e.y * abs(p.n.y) + e.z * abs(p.n.z);
		auto s = vx::dot(p.n, c) - p.d;

		return fabsf(s) <= r;
		return false;
	}*/
}

void InfluenceMap::createCell(__m128 vBoundsMin, __m128 vCellSize, const vx::uint3 &cellPosition, u32 index)
{
	__m128 vOffset = { (f32)cellPosition.x, (f32)cellPosition.y, (f32)cellPosition.z, 0.0f };

	auto vmin = _mm_fmadd_ps(vOffset, vCellSize, vBoundsMin);
	auto vmax = _mm_add_ps(vmin, vCellSize);

	auto vPosition = _mm_add_ps(vmax, vmin);
	vPosition = _mm_mul_ps(vPosition, vx::g_VXOneHalf);

	m_cellBounds[index].vmin = vmin;
	m_cellBounds[index].vmax = vmax;

	InfluenceCell c;
	_mm_storeu_ps(&c.m_position.x, vPosition);
	c.m_influence = 0.0f;
	c.m_offset = 0;
	c.m_count = 0;
	c.m_area = 0.0f;
	c.m_priority = 0.0f;
	m_cells[index] = c;
}

void InfluenceMap::createCells(const vx::uint3 &cellCount, u32 cellTotalCount, const vx::float3 &cellDim, const AABB &navBounds)
{
	__m128 vCellSize = { cellDim.x, cellDim.y, cellDim.z, 0 };
	auto vBoundsMin = vx::loadFloat(navBounds.min);

	m_cells = std::make_unique<InfluenceCell[]>(cellTotalCount);
	m_cellBounds = std::make_unique<AABB_SIMD[]>(cellTotalCount);

	u32 index = 0;
	vx::uint3 cellPosition;
	for (cellPosition.z = 0; cellPosition.z < cellCount.z; ++cellPosition.z)
	{
		for (cellPosition.y = 0; cellPosition.y < cellCount.y; ++cellPosition.y)
		{
			for (cellPosition.x = 0; cellPosition.x < cellCount.x; ++cellPosition.x)
			{
				createCell(vBoundsMin, vCellSize, cellPosition, index);
				++index;
			}
		}
	}
}

void InfluenceMap::createNodeIndicesAndSetCellData(const NavMesh &navMesh, u32 cellTotalCount)
{
	auto triangleCount = navMesh.getTriangleCount();
	auto navMeshTriangles = navMesh.getNavMeshTriangles();

	m_navNodeIndices = std::make_unique<u16[]>(triangleCount);
	u32 offset = 0;
	for (u32 i = 0; i < cellTotalCount; ++i)
	{
		auto &cellBounds = m_cellBounds[i];
		auto &cell = m_cells[i];

		AABB bounds;
		vx::storeFloat(&bounds.min, cellBounds.vmin);
		vx::storeFloat(&bounds.max, cellBounds.vmax);

		u32 count = 0;
		for (u32 j = 0; j < triangleCount; ++j)
		{
			auto &navMeshTriangle = navMeshTriangles[j];

			if (bounds.intersects(navMeshTriangle.m_triangle))
			{
				m_navNodeIndices[offset + count] = j;
				cell.m_area += navMeshTriangle.m_triangle.getArea();

				++count;
			}
		}

		cell.m_count = count;
		cell.m_offset = offset;

		offset += count;
	}
}

void InfluenceMap::initialize(const NavMesh &navMesh, f32 cellSize, f32 cellHeight)
{
	auto navBounds = navMesh.getBounds();
	auto size = navBounds.max - navBounds.min;

	vx::ushort3 cellCount;
	cellCount.x = (size.x + cellSize) / cellSize;
	cellCount.y = (size.y + cellHeight) / cellHeight;
	cellCount.z = (size.z + cellSize) / cellSize;

	auto cellDim = vx::float3(cellSize, cellHeight, cellSize);
	u32 cellTotalCount = cellCount.x * cellCount.y * cellCount.z;

	createCells(cellCount, cellTotalCount, cellDim, navBounds);

	createNodeIndicesAndSetCellData(navMesh, cellTotalCount);

	vx::float3 gridBoundsMin = navBounds.min;
	vx::float3 gridBoundsMax = navBounds.min + vx::float3(cellCount) * cellDim;

	m_center = (gridBoundsMax + gridBoundsMin) * 0.5f;
	m_cellCount = cellCount;

	vx::float3 totalSize = vx::float3(m_cellCount) * cellDim;

	m_voxelHalfDim = vx::float3(m_cellCount) / 2.0f;
	vx::float3 gridHalfSize = totalSize / 2.0f;
	vx::float3 gridCellSize = gridHalfSize / m_voxelHalfDim;
	m_invGridCellSize = 1.0f / gridCellSize;
}

void InfluenceMap::update(f32 dt)
{
	u32 cellCount = m_cellCount.x * m_cellCount.y * m_cellCount.z;

	// decrease influence by certain amount each second
	const f32 influenceDecrease = 0.75f;

	for (u32 i = 0; i < cellCount; ++i)
	{
		f32 newInfluence = m_cells[i].m_influence - (influenceDecrease * dt);

		m_cells[i].m_influence = vx::max(newInfluence, 0.0f);
	}
}

void InfluenceMap::updateActor(f32 dt, const vx::float3 &position)
{
	//const f32 maxRadius = 1.0f;
	const f32 actorInfluence = 2.0f;

	auto vPosition = vx::loadFloat(position);

	auto influence = dt * actorInfluence;

	u32 cellCount = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	for (u32 i = 0; i < cellCount; ++i)
	{
		auto &bounds = m_cellBounds[i];

		if (AABB_POINT::contains(bounds.vmin, bounds.vmax, vPosition))
		{
			m_cells[i].m_influence += influence;
			break;
		}
	}

	u8 nullbuffer[16];
	memset(nullbuffer, 0, 16);

	/*m_file.write(position);
	m_file.write(m_cells.get(), cellCount);
	m_file.write(nullbuffer);*/
}

const InfluenceCell& InfluenceMap::getCell(u32 x, u32 y, u32 z) const
{
	u32 index = x + m_cellCount.x * (y + m_cellCount.y * z);
	u32 maxIndex = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	index = std::min(index, maxIndex);

	return getCell(index);
}

const InfluenceCell& InfluenceMap::getCell(u32 index) const
{
	//u32 maxIndex = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	//index = std::min(index, maxIndex);

	return m_cells[index];
}

const InfluenceCell* InfluenceMap::getInfluenceCells() const
{
	return m_cells.get();
}

u32 InfluenceMap::getCellCount() const
{
	return m_cellCount.x * m_cellCount.y * m_cellCount.z;
}

s32 InfluenceMap::getClosestCellIndex_nocheck(const vx::float3 &position) const
{
	auto tmp = (position - m_center);
	vx::int3 cellPos = tmp * m_invGridCellSize + m_voxelHalfDim;

	return cellPos.x + m_cellCount.x * (cellPos.y + m_cellCount.y * cellPos.z);
}

u32 InfluenceMap::getClosestCellIndex(const vx::float3 &position) const
{
	s32 maxIndex = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	s32 index = getClosestCellIndex_nocheck(position);

	index = std::max(index, 0);
	index = std::min(index, maxIndex);

	return index;
}

vx::int3 InfluenceMap::getClosestCellPosition(const vx::float3 &position) const
{
	auto tmp = (position - m_center);
	vx::int3 cellPos = tmp * m_invGridCellSize + m_voxelHalfDim;

	//cellPos = vx::max(cellPos, vx::int3(0));
	cellPos = vx::clamp(cellPos, vx::int3(0), vx::int3(m_cellCount));

	return cellPos;
}

const u16* InfluenceMap::getNavNodeIndices() const
{
	return m_navNodeIndices.get();
}

bool InfluenceMap::isEmpty(u32 cellIndex) const
{
	return (m_cells[cellIndex].m_count == 0);
}

bool InfluenceMap::contains(u32 cellIndex, const vx::float3 &position) const
{
	auto vPosition = vx::loadFloat(position);
	auto &bounds = m_cellBounds[cellIndex];

	return AABB_POINT::contains(bounds.vmin, bounds.vmax, vPosition);
}

void InfluenceMap::getCells(const vx::float3 &p, f32 minRadius, f32 maxRadius, u32 maxCount, InfluenceCell* pCell, u32* count) const
{
	auto minRadius2 = minRadius * minRadius;
	f32 maxRadius2 = maxRadius * maxRadius;

	(*count) = 0;

	u32 cellCount = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	for (u32 i = 0; i < cellCount; ++i)
	{
		auto &cell = m_cells[i];

		auto dist2 = vx::distance2(cell.m_position, p);

		auto cmp = (dist2 > minRadius2) & (dist2 < maxRadius2);

		if (cmp && (cell.m_count != 0))
		{
			pCell[(*count)++] = cell;

			if ((*count) == maxCount)
				break;
		}
	}
}