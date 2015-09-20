#include "GpuVoxel.h"
#include "gpumath.h"

struct VSOutput
{
	float4 pos : SV_POSITION;
	uint2 texelPos : POSITION0;
	int3 voxelPosition : POSITION1;
	float3 wsPosition : POSITION2;
	float3 wsNormal : NORMAL0;
};

Texture3D<float4> g_voxelColor : register(t2);
Texture3D<uint> g_voxelOpacity : register(t3);

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel g_voxel;
};

static const float4 qRotations_normal[6] =
{
	{ 0.382683426, 0.000000000, 0.000000000, 0.923879504 },
	{ -0.382683426, 0.000000000, 0.000000000, 0.923879504 },
	{ 0.000000000, 0.382683426, 0.000000000, 0.923879504 },
	{ 0.000000000, -0.382683426, 0.000000000, 0.923879504 },
	{ 0.000000000, 0.000000000, 0.382683426, 0.923879504 },
	{ 0.000000000, 0.000000000, -0.382683426, 0.923879504 }
};

// 45° deg 2 axis
static const float4 qRotations_high[12] =
{
	{ -0.353553385, -0.146446601, -0.353553385, 0.853553355 },
	{ 0.353553385, 0.146446601, -0.353553385, 0.853553355 },
	{ -0.353553385, 0.146446601, 0.353553385, 0.853553355 },
	{ 0.353553385, -0.146446601, 0.353553385, 0.853553355 },

	{ -0.353553385, -0.353553385, -0.146446601, 0.853553355 },
	{ 0.353553385, -0.353553385, 0.146446601, 0.853553355 },
	{ 0.353553385, 0.353553385, -0.146446601, 0.853553355 },
	{ -0.353553385, 0.353553385, 0.146446601, 0.853553355 },

	{ 0.146446601, -0.353553385, -0.353553385, 0.853553355 },
	{ -0.146446601, 0.353553385, -0.353553385, 0.853553355 },
	{ -0.146446601, -0.353553385, 0.353553385, 0.853553355 },
	{ 0.146446601, 0.353553385, 0.353553385, 0.853553355 }
};

uint3 getAxis(in float3 direction)
{
	uint3 axis = uint3(0, 2, 4);
	uint3 offset = 0;

	offset.x = (direction.x < 0) ? 1 : 0;
	offset.y = (direction.y < 0) ? 1 : 0;
	offset.z = (direction.z < 0) ? 1 : 0;

	return axis + offset;
}

float getCoverage(uint mask)
{
	uint result = mask & 0x1 + (mask >> 1 & 0x1) + (mask >> 2 & 0x1) + (mask >> 3 & 0x1);

	return float(result) / 4.0;
}

float4 coneTrace(in float3 direction, in uint3 voxelPosition, in uint3 voxelDim, float coneRatio, float maxDist)
{
	uint3 axis = getAxis(-direction);
	uint3 wOffset = axis * g_voxel.dim;

	uint3 posX = voxelPosition + uint3(0, 0, wOffset.x);
	uint3 posY = voxelPosition + uint3(0, 0, wOffset.y);
	uint3 posZ = voxelPosition + uint3(0, 0, wOffset.z);

	float3 offset = direction * 1.0 / float3(voxelDim);
	float3 offset1 = direction * 1.0 / float3(g_voxel.dim, g_voxel.dim, g_voxel.dim);

	float3 texCoord = float3(voxelPosition) / g_voxel.dim + offset1;
	float3 texCoordX = float3(posX) / float3(voxelDim)+offset;
	float3 texCoordY = float3(posY) / float3(voxelDim)+offset;
	float3 texCoordZ = float3(posZ) / float3(voxelDim)+offset;

	float3 sampleWeights = abs(direction);
	float invWeight = 1.0 / (sampleWeights.x + sampleWeights.y + sampleWeights.z);
	sampleWeights *= invWeight;

	float minDiameter = 1.0 / g_voxel.dim;
	float minVoxelDiameterInv = g_voxel.dim;

	float4 accum = 0;
	float dist = maxDist - minDiameter;
	float traveledDist = minDiameter;

	while (accum.a < 1.0 && dist > 0.0)
	{
		float sampleDiameter = max(minDiameter, coneRatio * traveledDist);
		//sampleDiameter = minDiameter;
		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
		sampleLOD = clamp(sampleLOD, 0.0, 5.0);

		int mipLevel = sampleLOD;

		float4 color0 = g_voxelColor.Load(int4(int3(texCoordX * voxelDim) >> mipLevel, mipLevel));
		float4 color1 = g_voxelColor.Load(int4(int3(texCoordY * voxelDim) >> mipLevel, mipLevel));
		float4 color2 = g_voxelColor.Load(int4(int3(texCoordZ * voxelDim) >> mipLevel, mipLevel));

		/*uint opacityMask = g_voxelOpacity.Load(int4(texCoord, 0));
		const uint tmp = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
		uint maskX = (opacityMask >> axis.x * 4) & tmp;
		uint maskY = (opacityMask >> axis.y * 4) & tmp;
		uint maskZ = (opacityMask >> axis.z * 4) & tmp;

		float coverageX = getCoverage(maskX);
		float coverageY = getCoverage(maskY);
		float coverageZ = getCoverage(maskZ);

		color0.a = coverageX;
		color1.a = coverageY;
		color2.a = coverageZ;*/

		float4 sampleColor = color0 * sampleWeights.x + color1 * sampleWeights.y + color2 * sampleWeights.z;
		float sampleWeight = (1.0 - accum.w);

		accum += sampleColor * sampleWeight;

		texCoordX += offset;
		texCoordY += offset;
		texCoordZ += offset;
		texCoord += offset1;

		dist -= sampleDiameter;
		traveledDist += sampleDiameter;
	}

	return accum;
}

float4 main(VSOutput input) : SV_TARGET
{
	uint wDim = g_voxel.dim * 6;
	uint3 voxelDim = uint3(g_voxel.dim, g_voxel.dim, wDim);

	float4 accum = 0;

	float maxDist = 0.3;
	float coneRatio = 1.0;
	const int sum = 12;
	for (int i = 0; i < sum; ++i)
	{
		float3 direction = quaternionRotation(input.wsNormal, qRotations_high[i]);
		accum += coneTrace(direction, input.voxelPosition, voxelDim, coneRatio, maxDist);
	}

	accum /= sum;

	return float4(accum.rgb, 1.0);
}