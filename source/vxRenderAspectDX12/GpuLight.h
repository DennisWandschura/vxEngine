#include "Gpu.h"

struct GpuLight
{
	float4 position;
	float falloff;
	float lumen;
	float padding[2];
};