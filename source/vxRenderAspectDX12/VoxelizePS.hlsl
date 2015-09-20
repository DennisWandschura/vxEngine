#include "GpuVoxel.h"
#include "gpumath.h"
#include "GpuCameraBufferData.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	uint offset : BLLENDINDICES0;
	uint axis : BLENDINDICES1;
	uint material : BLENDINDICES0;
	float2 texCoords : TEXCOORD0;
};

Texture2DArray g_textureSrgba : register(t2);

RWTexture3D<uint> g_voxelTextureOpacity : register(u0);

SamplerState g_sampler : register(s0);

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

static const float3 g_swizzle[]=
{
	float3(0, 0, 1),
	float3(0, 1, 0),
	float3(1, 0, 0),

	float3(1, 0, 0),
	float3(0, 0, 1),
	float3(0, 1, 0),

	float3(1, 0, 0),
	float3(0, 1, 0),
	float3(0, 0, 1)
};

void writeOpacity(uint3 gridPosition, int axis, uint coverageMask)
{
	const uint tmp = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
	coverageMask = coverageMask & tmp;

	uint mask = (coverageMask << axis * 4);

	uint oldValue;
	InterlockedOr(g_voxelTextureOpacity[(uint3)gridPosition], mask, oldValue);
}

uint3 getTextureSlices(uint packedSlices)
{
	uint3 result;
	result.x = packedSlices & 0xff;
	result.y = (packedSlices >> 8) & 0xff;
	result.z = (packedSlices >> 16) & 0xff;

	return result;
}

float3 swizzleNormal(float3 normal, uint axis)
{
	uint offset = (axis / 2) * 3;

	float3 result;
	result.x = dot(normal, g_swizzle[offset]);
	result.y = dot(normal, g_swizzle[offset+1]);
	result.z = dot(normal, g_swizzle[offset+2]);

	return result;
}

uint getPackedNormal(float3 normal, uint axis)
{
	float3 swizzledNormal = swizzleNormal(normal, axis) * 0.5 + 0.5;
	swizzledNormal *= UINT10_MAX;

	uint packedNormal = uint(swizzledNormal.x) | (uint(swizzledNormal.y) << 10) | (uint(swizzledNormal.z) << 20);
	return packedNormal;
}

void main(GSOutput input, uint coverage : SV_Coverage)
{
	float3 offset = input.wsPosition * voxel.invGridCellSize;
	int3 coords = int3(offset) + voxel.halfDim;

	if (coords.x < 0 || coords.y < 0 || coords.z < 0 ||
		coords.x >= voxel.dim || coords.y >= voxel.dim || coords.z >= voxel.dim)
		discard;

	uint3 textureSlices = getTextureSlices(input.material);
	float4 diffuseColor = g_textureSrgba.Sample(g_sampler, float3(input.texCoords, float(textureSlices.x)));

	writeOpacity(coords, input.axis, coverage);

	//coords.z += input.offset;

	//float3 color = diffuseColor.rgb;
	//float luminance = getLuminance(color);

	//uint packedColor = packR8G8B8A8(float4(color, luminance));
	//uint oldValue;
	//InterlockedMax(g_voxelTextureDiffuse[(uint3)coords], packedColor, oldValue);
}