#include "GpuMath.h"

struct Output
{
	float4 diffuseSlice : SV_TARGET0;
	half2 normalSlice : SV_TARGET1;
	half2 velocitySlice : SV_TARGET2;
};

struct PSInput
{
	float4 position : SV_POSITION;
	//float4 positionPrev : POSITION1;
	//float3 vsPosition : POSITION2;
	float3 vsNormal : NORMAL0;
	//float3 vsTangent : TANGENT0;
	//float3 vsBitangent : BITANGENT0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
	uint slice : SV_RenderTargetArrayIndex;
};

struct Material
{
	uint textureSlices;
};

Texture2DArray g_textureSrgba : register(t2);
Texture2DArray g_textureRgba : register(t3);
SamplerState g_sampler : register(s0);

uint3 getTextureSlices(uint packedSlices)
{
	uint3 result;
	// diffuse
	result.x = packedSlices & 0xff;
	// normal
	result.y = (packedSlices >> 8) & 0xff;
	// surface
	result.z = (packedSlices >> 16) & 0xff;

	return result;
}

Output main(PSInput input)
{
	uint3 textureSlices = getTextureSlices(input.material);

	float4 diffuseColor = g_textureSrgba.Sample(g_sampler, float3(input.texCoords, float(textureSlices.x)));
	float3 normalMap = g_textureRgba.Sample(g_sampler, float3(input.texCoords, float(textureSlices.y))).xyz;

	float3x3 tbnMatrix;

	//float linearViewZ = length(input.vsPosition);
	float2 compressedNormal = encodeNormal(input.vsNormal);

	//float2 a = (input.position.xy / input.position.w) * 0.5 + 0.5;
	//float2 b = (input.positionPrev.xy / input.positionPrev.w) * 0.5 + 0.5;

	Output output;
	output.diffuseSlice = diffuseColor;
	output.normalSlice = compressedNormal;
	output.velocitySlice = float2(0, 0);//pow((a - b) * 0.5 + 0.5, 3.0);

	return output;
}