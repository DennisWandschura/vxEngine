#include "gpushading.h"
#include "gpumath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint slice : SV_RenderTargetArrayIndex;
	uint material : BLENDINDICES0;
	float distanceToLight : BLENDINDICES1;
	float lightFalloff : BLENDINDICES2;
	float lightLumen : BLENDINDICES3;
};

struct PSOut
{
	float zDepth : SV_TARGET0;
	float3 albedoColor : SV_TARGET1;
	half2 normals : SV_TARGET2;
};

Texture2DArray g_srgb : register(t2);
SamplerState g_sampler : register(s0);

uint3 getTextureSlices(uint packedSlices)
{
	uint3 result;
	result.x = packedSlices & 0xff;
	result.y = (packedSlices >> 8) & 0xff;
	result.z = (packedSlices >> 16) & 0xff;

	return result;
}

PSOut main(GSOutput input)
{
	float falloffValue = getFalloff(input.distanceToLight, input.lightFalloff);
	uint3 textureSlices = getTextureSlices(input.material);

	PSOut output;
	output.zDepth = input.distanceToLight / input.lightFalloff;
	output.albedoColor = g_srgb.Sample(g_sampler, float3(input.texCoords, textureSlices.x)).rgb;
	output.normals = encodeNormal(input.vsNormal);
	
	return output;
}