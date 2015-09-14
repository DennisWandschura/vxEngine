#include "Gpu.h"

struct GpuShadowTransform
{
	float4x4 pvMatrix[6];
};

struct GpuShadowTransformReverse
{
	float4x4 invPvMatrix[6];
};