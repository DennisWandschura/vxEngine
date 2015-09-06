#include "gpushading.h"
#include "gpumath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint slice : SV_RenderTargetArrayIndex;
	float distanceToLight : BLENDINDICES1;
	float lightFalloff : BLENDINDICES2;
	float lightLumen : BLENDINDICES3;
};

struct PSOut
{
	float zDepth : SV_TARGET0;
	half lightIntensity : SV_TARGET1;
	half2 normals : SV_TARGET2;
};

PSOut main(GSOutput input)
{
	float falloffValue = getFalloff(input.distanceToLight, input.lightFalloff);

	PSOut output;
	output.zDepth = input.distanceToLight / input.lightFalloff;
	output.lightIntensity = falloffValue * input.lightLumen / (4 * g_PI);
	output.normals = encodeNormal(input.vsNormal);
	
	return output;
}