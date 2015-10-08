#ifdef _VX_WINDOWS
#pragma once
#endif

#include "Gpu.h"

struct GpuLight
{
	float4 position;
	float falloff;
	float lumen;
	float padding[2];
};