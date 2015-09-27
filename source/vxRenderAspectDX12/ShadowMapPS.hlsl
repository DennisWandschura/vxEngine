#include "gpumath.h"
#include "gpushading.h"
#include "gpuvoxel.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	float3 lightPositionWS : POSITION1;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint slice : SV_RenderTargetArrayIndex;
	uint material : BLENDINDICES0;
	float lightFalloff : BLENDINDICES2;
	float lightLumen : BLENDINDICES3;
};

struct PSOut
{
	float2 zDepth : SV_TARGET0;
	float3 albedoColor : SV_TARGET1;
	half4 normals : SV_TARGET2;
};

Texture2DArray g_srgb : register(t4);
SamplerState g_sampler : register(s0);

RWTexture3D<uint> g_voxelTextureDiffuse : register(u3);

cbuffer VoxelBuffer : register(b2)
{
	GpuVoxel g_voxel;
};

static const int g_voxelAxis[] =
{
	1, 0,
	3, 2,
	4, 5
};

uint3 getTextureSlices(uint packedSlices)
{
	uint3 result;
	result.x = packedSlices & 0xff;
	result.y = (packedSlices >> 8) & 0xff;
	result.z = (packedSlices >> 16) & 0xff;

	return result;
}

float2 getMoments(float z)
{
	float2 Moments;
	Moments.x = z;

	float dx = ddx(z);
	float dy = ddy(z);
	Moments.y = z*z + 0.25*(dx*dx + dy*dy);

	return Moments;
}

PSOut main(GSOutput input)
{
	uint3 textureSlices = getTextureSlices(input.material);

	float3 dirToLight = input.lightPositionWS - input.wsPosition;
	float distanceToLight = length(dirToLight);
	float3 l = dirToLight / distanceToLight;

	float z = distanceToLight / input.lightFalloff;

	float3 albedoColor = g_srgb.Sample(g_sampler, float3(input.texCoords, textureSlices.x)).rgb;

	float nDotL = max(dot(l, input.normal), 0);
	float lightIntensity = getLightIntensity(distanceToLight, input.lightFalloff, input.lightLumen);

	float3 shadedColor = albedoColor / g_PI * lightIntensity * nDotL;

	PSOut output;
	output.zDepth = getMoments(z);
	output.albedoColor = shadedColor;
	output.normals = half4(input.normal, 1);

	/*float3 offset = (input.wsPosition - g_voxel.gridCenter.xyz) * g_voxel.invGridCellSize;
	int3 coords = int3(offset)+g_voxel.halfDim;

	bool inGrid = true;
	if (coords.x < 0 || coords.y < 0 || coords.z < 0 ||
		coords.x >= g_voxel.dim || coords.y >= g_voxel.dim || coords.z >= g_voxel.dim)
		inGrid = false;

	if (inGrid)
	{
		float luminance = getLuminance(shadedColor);
		uint packledColor = packR8G8B8A8(float4(shadedColor, luminance));
		int3 wOffset = int3(0, 0, g_voxelAxis[input.slice] * g_voxel.dim);

		uint oldValue;
		InterlockedMax(g_voxelTextureDiffuse[coords + wOffset], packledColor, oldValue);
	}*/
	
	return output;
}