#include "InfluenceMap.h"
#include "NavMesh.h"
#include <random>
#include <algorithm>
#include "NavGraph.h"
#include "NavNode.h"

namespace
{
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
}

void InfluenceMap::init(const NavMesh &navMesh, const NavGraph &navGraph, F32 cellSize, F32 cellHeight)
{
	auto navBounds = navMesh.getBounds();
	//	auto vCount = navMesh.getVertexCount();
	//auto vertices = navMesh.getVertices();

	auto size = navBounds.max - navBounds.min;

	auto vBoundsMin = vx::loadFloat(navBounds.min);
	__m128 vCellSize = { cellSize, cellHeight, cellSize, 0 };

	vx::ushort3 cellCount;
	cellCount.x = (size.x + cellSize) / cellSize;
	cellCount.y = (size.y + cellHeight) / cellHeight;
	cellCount.z = (size.z + cellSize) / cellSize;

	U32 cellTotalCount = cellCount.x * cellCount.y * cellCount.z;

	m_cells = std::make_unique<InfluenceCell[]>(cellTotalCount);
	m_cellBounds = std::make_unique<CellBounds[]>(cellTotalCount);

	U32 index = 0;
	for (U32 z = 0; z < cellCount.z; ++z)
	{
		F32 fz = z;
		__m128 vz = _mm_load_ss(&fz);

		for (U32 y = 0; y < cellCount.y; ++y)
		{
			F32 fy = y;
			__m128 vy = _mm_load_ss(&fy);

			for (U32 x = 0; x < cellCount.x; ++x)
			{
				F32 fx = static_cast<F32>(x);

				__m128 vOffset = _mm_load_ss(&fx);
				// x, z
				vOffset = _mm_unpacklo_ps(vOffset, vz);
				// x, y, z
				vOffset = _mm_unpacklo_ps(vOffset, vy);

				auto vmin = _mm_fmadd_ps(vOffset, vCellSize, vBoundsMin);
				auto vmax = _mm_add_ps(vmin, vCellSize);

				auto vPosition = _mm_add_ps(vmax, vmin);
				vPosition = _mm_mul_ps(vPosition, vx::g_VXOneHalf);

				m_cellBounds[index].vmin = vmin;
				m_cellBounds[index].vmax = vmax;

				InfluenceCell c;
				_mm_store_ps(&c.position.x, vPosition);
				c.influence = 0.0f;
				c.offset = 0;
				c.count = 0;
				m_cells[index] = c;

				++index;
			}
		}
	}

	auto nodeCount = navGraph.getNodeCount();
	auto pNodes = navGraph.getNodes();
	m_navNodeIndices = std::make_unique<U16[]>(nodeCount);
	U32 offset = 0;
	for (U32 i = 0; i < cellTotalCount; ++i)
	{
		auto &cellBounds = m_cellBounds[i];
		auto &cell = m_cells[i];

		U32 count = 0;
		for (U32 j = 0; j < nodeCount; ++j)
		{
			auto &node = pNodes[j];
			auto p = vx::loadFloat(node.m_position);

			if (AABB_POINT::contains(cellBounds.vmin, cellBounds.vmax, p))
			{
				m_navNodeIndices[offset + count] = j;

				++count;
			}
		}

		cell.count = count;
		cell.offset = offset;

		offset += count;
	}

	vx::float3 gridBoundsMin = navBounds.min;
	vx::float3 gridBoundsMax = navBounds.min + vx::float3(cellCount) * vx::float3(cellSize, cellHeight, cellSize);

	m_center = (gridBoundsMax + gridBoundsMin) * 0.5f;
	m_cellCount = cellCount;

	vx::float3 cellDim(cellSize, cellHeight, cellSize);
	vx::float3 totalSize = vx::float3(m_cellCount) * cellDim;

	m_voxelHalfDim = vx::float3(m_cellCount) / 2.0f;
	vx::float3 gridHalfSize = totalSize / 2.0f;
	vx::float3 gridCellSize = gridHalfSize / m_voxelHalfDim;
	m_invGridCellSize = 1.0f / gridCellSize;
}

void InfluenceMap::update(F32 dt)
{
	U32 cellCount = m_cellCount.x * m_cellCount.y * m_cellCount.z;

	// decrease influence by certain amount each second
	const F32 influenceDecrease = 0.75f;

	for (U32 i = 0; i < cellCount; ++i)
	{
		F32 newInfluence = m_cells[i].influence - (influenceDecrease * dt);

		m_cells[i].influence = vx::max(newInfluence, 0.0f);
	}
}

void InfluenceMap::updateActor(F32 dt, const vx::float3 &position)
{
	//const F32 maxRadius = 1.0f;
	const F32 actorInfluence = 2.0f;

	auto vPosition = vx::loadFloat(position);

	auto influence = dt * actorInfluence;

	U32 cellCount = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	for (U32 i = 0; i < cellCount; ++i)
	{
		auto &bounds = m_cellBounds[i];

		if (AABB_POINT::contains(bounds.vmin, bounds.vmax, vPosition))
		{
			m_cells[i].influence += influence;
			break;
		}
	}

	U8 nullbuffer[16];
	memset(nullbuffer, 0, 16);

	/*m_file.write(position);
	m_file.write(m_cells.get(), cellCount);
	m_file.write(nullbuffer);*/
}

const InfluenceCell& InfluenceMap::getCell(U16 x, U16 y, U16 z) const
{
	U32 index = x + m_cellCount.x * (y + m_cellCount.y * z);

	return getCell(index);
}

const InfluenceCell& InfluenceMap::getCell(U32 index) const
{
	//U32 maxIndex = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	//index = std::min(index, maxIndex);

	return m_cells[index];
}

I32 InfluenceMap::getClosestCellIndex_nocheck(const vx::float3 &position) const
{
	vx::int3 cellPos = vx::fma((position - m_center), m_invGridCellSize, m_voxelHalfDim);

	return cellPos.x + m_cellCount.x * (cellPos.y + m_cellCount.y * cellPos.z);
}

U32 InfluenceMap::getClosestCellIndex(const vx::float3 &position) const
{
	I32 maxIndex = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	I32 index = getClosestCellIndex_nocheck(position);

	index = std::max(index, 0);
	index = std::min(index, maxIndex);

	return index;
}

const vx::float3& InfluenceMap::getClosestCellPosition(const vx::float3 &position) const
{
	auto index = getClosestCellIndex(position);

	return m_cells[index].position;
}

const U16* InfluenceMap::getNavNodeIndices() const
{
	return m_navNodeIndices.get();
}

U8 InfluenceMap::isEmpty(U32 cellIndex) const
{
	return (m_cells[cellIndex].count == 0);
}

U8 InfluenceMap::contains(U32 cellIndex, const vx::float3 &position) const
{
	auto vPosition = vx::loadFloat(position);
	auto &bounds = m_cellBounds[cellIndex];

	return AABB_POINT::contains(bounds.vmin, bounds.vmax, vPosition);
}

void InfluenceMap::getCells(const vx::float3 &p, F32 minRadius, F32 maxRadius, U32 maxCount, InfluenceCell* pCell, U32* count) const
{
	auto minRadius2 = minRadius * minRadius;
	F32 maxRadius2 = maxRadius * maxRadius;

	(*count) = 0;

	U32 cellCount = m_cellCount.x * m_cellCount.y * m_cellCount.z;
	for (U32 i = 0; i < cellCount; ++i)
	{
		auto &cell = m_cells[i];

		auto dist2 = vx::distance2(cell.position, p);

		auto cmp = (dist2 > minRadius2) & (dist2 < maxRadius2);

		if (cmp && (cell.count != 0))
		{
			pCell[(*count)++] = cell;

			if ((*count) == maxCount)
				break;
		}
	}
}