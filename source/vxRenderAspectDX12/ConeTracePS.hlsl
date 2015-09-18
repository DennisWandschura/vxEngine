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

uint3 getAxis(in float3 direction)
{
	uint3 axis = uint3(0, 2, 4);
	uint3 offset = 0;

	offset.x = (direction.x < 0) ? 1 : 0;
	offset.y = (direction.y < 0) ? 1 : 0;
	offset.z = (direction.z < 0) ? 1 : 0;

	return axis + offset;
}

float4 coneTrace(in float3 direction, in uint3 voxelPosition, in uint3 voxelDim)
{
	uint3 axis = getAxis(-direction);
	uint3 wOffset = axis * g_voxel.dim;

	uint3 posX = voxelPosition + uint3(0, 0, wOffset.x);
	uint3 posY = voxelPosition + uint3(0, 0, wOffset.y);
	uint3 posZ = voxelPosition + uint3(0, 0, wOffset.z);

	float3 offset = direction * 1.0 / float3(voxelDim);

	float3 texCoordX = float3(posX) / float3(voxelDim)+offset;
	float3 texCoordY = float3(posY) / float3(voxelDim)+offset;
	float3 texCoordZ = float3(posZ) / float3(voxelDim)+offset;

	float3 sampleWeights = abs(direction);
	float invWeight = 1.0 / (sampleWeights.x + sampleWeights.y + sampleWeights.z);
	sampleWeights *= invWeight;

	float maxDist = 0.3;
	float minDiameter = 1.0 / g_voxel.dim;
	float minVoxelDiameterInv = g_voxel.dim;
	float coneRatio = 1.0;

	float4 accum = 0;
	float dist = maxDist - minDiameter;

	while (accum.a < 1.0 && dist > 0.0)
	{
		float sampleDiameter = minDiameter;//max(minDiameter, coneRatio * dist);
		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);

		float4 color0 = g_voxelColor.Load(int4(int3(texCoordX * voxelDim), sampleLOD));
		float4 color1 = g_voxelColor.Load(int4(int3(texCoordY * voxelDim), sampleLOD));
		float4 color2 = g_voxelColor.Load(int4(int3(texCoordZ * voxelDim), sampleLOD));

		accum += color0 * sampleWeights.x + color1 * sampleWeights.y + color2 * sampleWeights.z;

		texCoordX += offset;
		texCoordY += offset;
		texCoordZ += offset;

		dist -= sampleDiameter;
	}

	return accum;
}

float4 main(VSOutput input) : SV_TARGET
{
	uint wDim = g_voxel.dim * 6;
	uint3 voxelDim = uint3(g_voxel.dim, g_voxel.dim, wDim);

	float4 accum = 0;

	float3 direction = quaternionRotation(input.wsNormal, qRotations_normal[0]);
	accum += coneTrace(direction, input.voxelPosition, voxelDim);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[1]);
	accum += coneTrace(direction, input.voxelPosition, voxelDim);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[2]);
	accum += coneTrace(direction, input.voxelPosition, voxelDim);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[3]);
	accum += coneTrace(direction, input.voxelPosition, voxelDim);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[4]);
	accum += coneTrace(direction, input.voxelPosition, voxelDim);

	direction = quaternionRotation(input.wsNormal, qRotations_normal[5]);
	accum += coneTrace(direction, input.voxelPosition, voxelDim);

	accum /= 6.0;

	return float4(accum.rgb, 1.0);
}