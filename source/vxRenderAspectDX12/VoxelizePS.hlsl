#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	uint offset : BLLENDINDICES0;
};

static const float3 g_luminanceVector = float3(0.2126, 0.7152, 0.0722);

RWTexture3D<float> g_voxelTextureOpacity : register(u0);
RWTexture3D<uint> g_voxelTextureDiffuse : register(u1);

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

void main(GSOutput input)
{
	float3 offset = input.wsPosition * voxel.invGridCellSize;
	int3 coords = int3(offset) + voxel.halfDim;

	if (coords.x < 0 || coords.y < 0 || coords.z < 0 ||
		coords.x >= voxel.dim || coords.y >= voxel.dim || coords.z >= voxel.dim)
		discard;

	coords.z += input.offset;

	g_voxelTextureOpacity[(uint3)coords] = 1.0;

	float3 color = 1.0;
	float luminance = dot(color, g_luminanceVector);

	uint4 colorU8 = uint4(color * 255.0, luminance * 255);

	uint packedColor = colorU8.x | (colorU8.y << 8) | (colorU8.z << 16) | (colorU8.w << 24);
	uint oldValue;
	InterlockedMax(g_voxelTextureDiffuse[(uint3)coords], packedColor, oldValue);
}