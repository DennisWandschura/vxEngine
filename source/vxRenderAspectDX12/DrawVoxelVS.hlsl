#include "GpuVoxel.h"

struct VSOut
{
	uint3 voxelPos : BLENDINCIES0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

VSOut main( uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint size = voxel.dim;

	uint x = vertexID;
	uint yz = instanceID;

	VSOut output;
	output.voxelPos.x = x;
	output.voxelPos.y = yz % size;
	output.voxelPos.z = yz / size;

	return output;
}