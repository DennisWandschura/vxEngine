#include "GpuCameraBufferData.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

Texture2DArray g_depthSlice : register(t0);
SamplerState g_sampler : register(s0);

float main(GSOutput input) : SV_TARGET
{
	float depth = g_depthSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).r;
	float zNear = cameraBuffer.zNear;
	float zFar = cameraBuffer.zFar;

	float c0 = zNear * zFar;
	float c1 = zNear - zFar;
	float c2 = zFar;

	float z = c0 / (depth * c1 + c2);

	return z;
}