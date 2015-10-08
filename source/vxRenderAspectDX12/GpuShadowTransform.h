#ifdef _VX_WINDOWS
#pragma once
#endif

#include "Gpu.h"

struct GpuShadowTransform
{
	float4x4 pvMatrix[6];
};

struct GpuShadowTransformReverse
{
	float4x4 invPvMatrix[6];
};