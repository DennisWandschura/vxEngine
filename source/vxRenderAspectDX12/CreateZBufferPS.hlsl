#include "GpuCameraBufferData.h"

struct PSOutput
{
	float zBuffer0 : SV_TARGET0;
	float zBuffer1 : SV_TARGET1;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

cbuffer CameraStaticBuffer : register(b0)
{
	GpuCameraStatic cameraStatic;
};

Texture2DArray g_depthSlice : register(t0);
SamplerState g_sampler : register(s0);

PSOutput main(GSOutput input)
{
	float depth0 = g_depthSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).r;
	float depth1 = g_depthSlice.Sample(g_sampler, float3(input.texCoords, 1.0)).r;
	float zNear = cameraStatic.zNear;
	float zFar = cameraStatic.zFar;

	float c0 = zNear * zFar;
	float c1 = zNear - zFar;
	float c2 = zFar;

	float z0 = c0 / (depth0 * c1 + c2) / zFar;
	float z1 = c0 / (depth1 * c1 + c2) / zFar;

	PSOutput output;
	output.zBuffer0 = z0;
	output.zBuffer1 = z1;

	return output;
}