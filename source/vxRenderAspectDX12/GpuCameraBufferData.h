#include "Gpu.h"

struct GpuCameraBufferData
{
	float4 position;
	float4x4 pvMatrix;
	float4x4 pvMatrixPrev;
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4 projInfo;
	float zNear;
	float zFar;
};