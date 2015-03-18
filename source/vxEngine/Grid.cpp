#include "Grid.h"

U32 Cell::getCellIndex(const vx::float3 &p, const vx::float3 &center, const vx::uint3 &cellCount, F32 cellSize)
{
	auto vPosition = vx::loadFloat(&p);
	auto vGridCenter = vx::loadFloat(&center);

	__m128 vCellCount = { F32(cellCount.x), F32(cellCount.y), F32(cellCount.z), 0 };
	__m128 vCellSize = { cellSize, cellSize, cellSize, 0 };
	auto vTotalSize = _mm_mul_ps(vCellCount, vCellSize);

	// range -0.5 ... 0.5
	auto vPos = _mm_sub_ps(vPosition, vGridCenter);
	vPos = _mm_div_ps(vPos, vTotalSize);

	// range 0 ... 1
	vPos = _mm_add_ps(vPos, vx::g_VXOneHalf);

	// clamp to 0..1
	vPos = _mm_max_ps(vPos, vx::g_VXZero);
	vPos = _mm_min_ps(vPos, vx::g_VXOne);

	auto vIndex = _mm_mul_ps(vPos, vTotalSize);
	vIndex = _mm_div_ps(vIndex, vCellSize);

	__m128i vIndexI = _mm_cvttps_epi32(vIndex);
	vIndex = _mm_round_ps(vIndex, 0x1);

	// convert 3d index to 1d
	vx::int4 tmp;
	vx::storeInt(&tmp, vIndexI);

	U32 index = tmp.x + cellCount.x * (tmp.y + cellCount.y * tmp.z);
	return index;
}