#pragma once

#include <vxLib/math/Vector.h>
#include "AABB.h"

struct Cell
{
	U32 count;
	U32 offset;

	static U32 getCellIndex(const vx::float3 &p, const vx::float3 &center, const vx::uint3 &cellCount, F32 cellSize);
};