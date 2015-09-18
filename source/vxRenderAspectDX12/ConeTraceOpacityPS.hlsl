#include "gpuvoxel.h"
#include "gpumath.h"

struct VSOutput
{
	float4 pos : SV_POSITION;
	uint2 texelPos : POSITION0;
	int3 voxelPosition : POSITION1;
	float3 wsPosition : POSITION2;
	float3 wsNormal : NORMAL0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel g_voxel;
};

Texture3D<uint> g_opacity : register(t2);

static const uint g_maxMip = 3;

// 45° deg 1 axis
static const float4 qRotations_normal[6] =
{
	{ 0.382683426, 0.000000000, 0.000000000, 0.923879504 },
	{ -0.382683426, 0.000000000, 0.000000000, 0.923879504 },
	{ 0.000000000, 0.382683426, 0.000000000, 0.923879504 },
	{ 0.000000000, -0.382683426, 0.000000000, 0.923879504 },
	{ 0.000000000, 0.000000000, 0.382683426, 0.923879504 },
	{ 0.000000000, 0.000000000, -0.382683426, 0.923879504 }
};

int3 getGridPosition(in float3 wsPosition, int mip)
{
	float3 offset = (wsPosition.xyz - g_voxel.gridCenter.xyz) * g_voxel.invGridCellSize;
	int3 voxelPosition = int3(offset) + g_voxel.halfDim;

	return voxelPosition;
}

uint getOpacityMask(in float3 normal, out uint3 axis)
{
	normal = -normal;

	uint dirx = (normal.x >= 0) ? 0 : 1;
	uint diry = (normal.y >= 0) ? 2 : 3;
	uint dirz = (normal.z >= 0) ? 4 : 5;

	axis.x = dirx;
	axis.y = diry;
	axis.z = dirz;

	return (1 << dirx) | (1 << diry) | (1 << dirz);
}

bool testOpacity(int3 voxelPos, int mipLevel, uint opacityMask)
{
	voxelPos = voxelPos >> mipLevel;
	uint mask = g_opacity.Load(int4(voxelPos, mipLevel));
	uint result = mask & opacityMask;

	return (result != 0);
}

bool test(int3 currentVoxelPosition, uint opacityMask)
{
	bool occluded = false;

	if(testOpacity(currentVoxelPosition, 3, opacityMask))
	{
		if (testOpacity(currentVoxelPosition, 2, opacityMask))
		{
			if (testOpacity(currentVoxelPosition, 1, opacityMask))
			{
				if (testOpacity(currentVoxelPosition, 0, opacityMask))
				{
					occluded = true;
				}
			}
		}
	}

	return occluded;
}

int getVoxelIndex(int3 voxelPos)
{
	return voxelPos.x + g_voxel.dim * (voxelPos.y + g_voxel.dim * voxelPos.z);
}

float getOcclusion(in float3 gridMin, in float3 gridMax, in float3 wsPosition, in float3 direction, float maxDist)
{
	float stepSize = g_voxel.gridCellSize;
	float3 absDir = abs(direction);

	float3 offset = stepSize * direction;

	float3 currentPosition = wsPosition + offset * 0.5;
	float3 endPosition = wsPosition + direction * maxDist;
	endPosition = max(endPosition, gridMin);
	endPosition = min(endPosition, gridMax);

	float3 dir = endPosition - currentPosition;
	float distanceToTravel = length(dir);
	float len = distanceToTravel;

	uint3 axis;
	uint opacityMask = getOpacityMask(direction, axis);

	bool occluded = false;

	float occlusion = 1.0;
	int3 currentVoxelPosition = getGridPosition(currentPosition, 0);
	int lastVoxelIndex = getVoxelIndex(currentVoxelPosition);
	while (len > 0.0)
	{
		currentVoxelPosition = getGridPosition(currentPosition, 0);
		int voxelIndex = getVoxelIndex(currentVoxelPosition);

		if (voxelIndex != lastVoxelIndex)
		{
			occluded = test(currentVoxelPosition, opacityMask);
			if (occluded)
			{
				//uint xx = (axismask >> axis.x) & 0x1;
				//uint yy = (axismask >> axis.y) & 0x1;
				//uint zz = (axismask >> axis.z) & 0x1;

				//float3 weights = absDir * float3(xx, yy, zz);

				/*int bb = 0;
				bb += testOpacity(currentVoxelPosition + int3(1, 0, 0), 0, opacityMask);
				bb += testOpacity(currentVoxelPosition + int3(1, 1, 0), 0, opacityMask);
				bb += testOpacity(currentVoxelPosition + int3(0, 1, 0), 0, opacityMask);

				bb += testOpacity(currentVoxelPosition + int3(0, 0, 1), 0, opacityMask);
				bb += testOpacity(currentVoxelPosition + int3(1, 0, 1), 0, opacityMask);
				bb += testOpacity(currentVoxelPosition + int3(1, 1, 1), 0, opacityMask);
				bb += testOpacity(currentVoxelPosition + int3(0, 1, 1), 0, opacityMask);

				float sum = (occluded + bb);
				sum /= 8.0;*/

				occlusion = 0.0;// -sum;
				break;
			}

			lastVoxelIndex = voxelIndex;
		}

		currentPosition += offset;
		len -= stepSize;
	}

	return occlusion;
}

float4 main(VSOutput input) : SV_TARGET
{
	float maxDist = 1.5;//max(2.0, g_voxel.mip[0].gridCellSize);

	float gridHalfSize = g_voxel.gridCellSize * g_voxel.halfDim;
	float3 gridMax = g_voxel.gridCenter.xyz + gridHalfSize;
	float3 gridMin = g_voxel.gridCenter.xyz - gridHalfSize;

	//float tmp = traveledDistance / distanceToTravel;
	float occlusion = 0.0;

	float3 direction = quaternionRotation(input.wsNormal, qRotations_normal[0]);
	occlusion += getOcclusion(gridMin, gridMax, input.wsPosition, direction, maxDist);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[1]);
	occlusion += getOcclusion(gridMin, gridMax, input.wsPosition, direction, maxDist);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[2]);
	occlusion += getOcclusion(gridMin, gridMax, input.wsPosition, direction, maxDist);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[3]);
	occlusion += getOcclusion(gridMin, gridMax, input.wsPosition, direction, maxDist);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[4]);
	occlusion += getOcclusion(gridMin, gridMax, input.wsPosition, direction, maxDist);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[5]);
	occlusion += getOcclusion(gridMin, gridMax, input.wsPosition, direction, maxDist);

	occlusion /= 6.0;

	return float4(occlusion, occlusion, occlusion, 1);
}