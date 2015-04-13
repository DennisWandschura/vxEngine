#pragma once

#include <vxLib/math/Vector.h>

struct GpuFunctions
{
	static vx::uint2 __vectorcall packQRotation(const __m128 qRotation);
};