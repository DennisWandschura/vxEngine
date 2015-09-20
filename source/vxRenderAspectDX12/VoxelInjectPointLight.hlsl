#include "gpulight.h"
#include "gpuvoxel.h"

RWTexture3D<float4> g_voxelTextureEmmitance : register(u0);
StructuredBuffer<GpuLight> g_lights;

cbuffer VoxelBuffer
{
	GpuVoxel g_voxel;
};

bool isPointInGrid(int3 position)
{
	bool result = true;
	if (position.x < 0 || position.y < 0 || position.z < 0 ||
		position.x >= g_voxel.dim || position.y >= g_voxel.dim || position.z >= g_voxel.dim)
		result = false;

	return result;
}

void main(uint lightIndex : SV_VertexID)
{
	float4 lightPosition = g_lights[lightIndex].position;
	float3 voxelCenter = g_voxel.gridCenter.xyz;

	// 5 cm surface radius
	float lightSurfaceRadius = 0.05;

	float3 pmin = lightPosition.xyz - lightSurfaceRadius;
	float3 pmax = lightPosition.xyz + lightSurfaceRadius;

	float3 offset = (pmin - voxelCenter) * g_voxel.invGridCellSize;
	int3 lightVoxelPositionMin = int3(offset)+g_voxel.halfDim;

	offset = (pmax - voxelCenter) * g_voxel.invGridCellSize;
	int3 lightVoxelPositionMax = int3(offset)+g_voxel.halfDim;

	int3 dim = lightVoxelPositionMax - lightVoxelPositionMin;

	for (int z = 0; z <= dim.z; ++z)
	{
		for (int y = 0; y <= dim.y; ++y)
		{
			for (int x = 0; x <= dim.x; ++x)
			{
				int3 currentPosition = lightVoxelPositionMin + int3(x, y, z);

				if (isPointInGrid(currentPosition))
				{
					g_voxelTextureEmmitance[currentPosition] = float4(1, 1, 1, 1);
				}
			}
		}
	}
}