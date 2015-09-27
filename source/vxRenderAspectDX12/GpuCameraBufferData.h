#include "Gpu.h"

struct GpuCameraBufferData
{
	float4x4 pvMatrix;
	float4x4 pvMatrixPrev;
	float4x4 viewMatrix;
	float4x4 invViewMatrix;
	float4 position;
};

struct GpuCameraStatic
{
	float4x4 orthoMatrix;
	float4x4 projMatrix;
	float4x4 invProjMatrix;
	float4 projInfo;
	float zNear;
	float zFar;
	float2 resolution;
};