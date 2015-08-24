#include "Gpu.h"

struct GpuCameraBufferData
{
	float4 position;
	float4x4 pvMatrix;
	float4x4 pvMatrixPrev;
	float4x4 viewMatrix;
};