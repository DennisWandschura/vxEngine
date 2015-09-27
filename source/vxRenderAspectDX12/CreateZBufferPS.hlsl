#include "GpuCameraBufferData.h"

struct PSOutput
{
	float zBuffer0 : SV_TARGET0;
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

Texture2D<float2> g_depthSlice : register(t0);
SamplerState g_sampler : register(s0);

PSOutput main(GSOutput input)
{
	float depth0 = g_depthSlice.Sample(g_sampler, input.texCoords).r;
	float zNear = cameraStatic.zNear;
	float zFar = cameraStatic.zFar;

	float c0 = zNear * zFar;
	float c1 = zNear - zFar;
	float c2 = zFar;

	float z0 = c0 / (depth0 * c1 + c2) / zFar;

	PSOutput output;
	output.zBuffer0 = z0;

	return output;
}