#include "GpuLight.h"
#include "GpuMath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

cbuffer LightCountBuffer : register(b0)
{
	uint g_lightCount;
};

StructuredBuffer<GpuLight> s_lights : register(t3);

Texture2DArray g_diffuseSlice : register(t0);
Texture2DArray g_normalSlice : register(t1);
Texture2DArray g_velocitySlice : register(t2);
SamplerState g_sampler : register(s0);

float4 main(GSOutput input) : SV_TARGET
{
	float3 diffuseColor = g_diffuseSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).rgb;
	float2 packedNormal = g_normalSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).rg;

	float3 vsNormal = decodeNormal(packedNormal);

	const float invGamma = 1.0 / 2.2;
	diffuseColor = pow(diffuseColor, float3(invGamma, invGamma, invGamma));

	return float4(diffuseColor, 1.0f);
}