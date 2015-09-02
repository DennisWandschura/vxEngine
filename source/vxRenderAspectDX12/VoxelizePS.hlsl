#include "GpuVoxel.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	uint offset : BLLENDINDICES0;
};

RWTexture3D<uint> g_voxelTexture : register(u0);

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

void main(GSOutput input)
{
	int3 coords = int3(input.wsPosition * voxel.invGridCellSize) + voxel.halfDim;

	if (coords.x < 0 || coords.y < 0 || coords.z < 0 ||
		coords.x >= voxel.dim || coords.y >= voxel.dim || coords.z >= voxel.dim)
		discard;

	coords.z += input.offset;

	g_voxelTexture[(uint3)coords] = 1;
}