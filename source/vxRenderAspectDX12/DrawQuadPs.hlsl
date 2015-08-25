#include "GpuMath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2DArray g_diffuseSlice : register(t0);
Texture2DArray g_normalVelocitySlice : register(t1);
SamplerState g_sampler : register(s0);

float4 main(GSOutput input) : SV_TARGET
{
	float3 diffuseColor = g_diffuseSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).rgb;
	float4 packedNormalVelocity = g_normalVelocitySlice.Sample(g_sampler, float3(input.texCoords, 0.0));

	float3 vsNormal = decodeNormal(packedNormalVelocity.rg);
	float2 velocity = packedNormalVelocity.ba;

	const float invGamma = 1.0 / 2.2;
	diffuseColor = pow(diffuseColor, float3(invGamma, invGamma, invGamma));

	return float4(diffuseColor, 1.0f);
}