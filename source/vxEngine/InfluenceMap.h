#pragma once

class NavMesh;
class NavGraph;

#include <vxLib/math/Vector.h>
#include <vector>
#include "AABB.h"
#include <memory>

struct InfluenceCell
{
	vx::float3 position;
	F32 influence;
	U16 offset;
	U16 count;
};

class InfluenceMap
{
	struct CellBounds
	{
		__m128 vmin, vmax;
	};

	std::unique_ptr<CellBounds[]> m_cellBounds;
	std::unique_ptr<InfluenceCell[]> m_cells; // 8
	// influence cell data
	std::unique_ptr<U16[]> m_navNodeIndices;
	vx::float3 m_center; // 12
	vx::float3 m_invGridCellSize; // 12
	vx::float3 m_voxelHalfDim; // 12
	vx::ushort3 m_cellCount; // 6

public:
	void init(const NavMesh &navMesh, const NavGraph &navGraph, F32 cellSize, F32 cellHeight);

	void update(F32 dt);
	void updateActor(F32 dt, const vx::float3 &position);

	const InfluenceCell& getCell(U16 x, U16 y, U16 z) const;
	const InfluenceCell& getCell(U32 index) const;

	I32 getClosestCellIndex_nocheck(const vx::float3 &position) const;
	U32 getClosestCellIndex(const vx::float3 &position) const;
	const vx::float3& getClosestCellPosition(const vx::float3 &position) const;
	const U16* getNavNodeIndices() const;

	U8 isEmpty(U32 cellIndex) const;
	U8 contains(U32 cellIndex, const vx::float3 &position) const;

	void getCells(const vx::float3 &p, F32 minRadius, F32 maxRadius, U32 maxCount, InfluenceCell* pCells, U32* count) const;
};