#include "GpuVoxel.h"

struct GSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	int3 voxelPosition : POSITION0;
	uint rotationIndex : BLENDINDICES0;
	uint offsetZ : BLENDINDICES1;
};

static const float3x3 rotationMatrices[4] =
{
	float3x3(float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1)),
	// x 45
	float3x3(float3(1, 0, 0), float3(0, 0.707106709, -0.707106709), float3(0, 0.707106709, 0.707106709)),
	// x -45
	//float3x3(float3(1, 0, 0), float3(0, 0.707106709, 0.707106709), float3(0, -0.707106709, 0.707106709)),

	// y 45
	float3x3(float3(0.707106709, 0, 0.707106709), float3(0, 1, 0), float3(-0.707106709, 0, 0.707106709)),
	// y -45
	//float3x3(float3(0.707106709, 0, -0.707106709), float3(0, 1, 0), float3(0.707106709, 0, 0.707106709)),

	// z 45
	float3x3(float3(0.707106709, -0.707106709, 0), float3(0.707106709, 0.707106709, 0), float3(0, 0, 1))
	// z -45
	//float3x3(float3(0.707106709, 0.707106709, 0), float3(-0.707106709, 0.707106709, 0), float3(0, 0, 1))
};

// Y = 0.2126 R + 0.7152 G + 0.0722 B
static const float3 g_luminanceVector = float3(0.2126, 0.7152, 0.0722);

static const uint g_maxMipLevel = 1;

Texture3D<uint> g_voxelTextureDiffuse : register(t2);

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

float4 unpackColor(uint packedColor)
{
	uint4 color = uint4(packedColor, packedColor >> 8, packedColor >> 16, packedColor >> 24) & 0xff;

	float4 unpackedColor = float4(color) / 255.0;
	return unpackedColor;
}

float4 main(GSOutput input) : SV_TARGET
{
	if (input.voxelPosition.x < 0 || input.voxelPosition.x >= voxel.dim ||
		input.voxelPosition.y < 0 || input.voxelPosition.y >= voxel.dim ||
		input.voxelPosition.z < 0 || input.voxelPosition.z >= voxel.dim)
		discard;

	uint3 voxelDim = uint3(voxel.dim, voxel.dim, voxel.dim * 6);

	//float3x3 rotationMatrix = rotationMatrices[input.rotationIndex];

	float3 texelSize = 1.0 / float3(voxelDim);


	float3 coords = input.voxelPosition * texelSize;

	// +x 0 1
	// -x 1 2
	// +y 2 3
	// -y 3 4
	// +z 4 5
	// -z 5 6
	float minCoordZ = (input.offsetZ / voxelDim.z);
	float maxCoordZ = minCoordZ + 1.0;
	float3 minCoords = float3(0, 0, minCoordZ);
	float3 maxCoords = float3(1, 1, maxCoordZ);

	float3 destCoords = coords + input.normal * 1000;
	destCoords = clamp(destCoords, minCoords, maxCoords);

	float maxDist = 0.25;
	maxDist = min(maxDist, length(destCoords - coords));

	//float3 stepSize = texelSize;
	//float stepSizeLen = length(stepSize);

	//float3 destination = currentPosition + offset * 1000.0;
	//destination = clamp(destination, minCoordZ, maxCoordZ);

	//float distanceToMax = length(destination - currentPosition);
	//uint sampleCount = 50;//(distanceToMax / stepSizeLen);
	//
	//uint maxSampleCount = 50;
	//float3 currentPosition = coords + offset;
	//uint sampleCount = distanceToMax / offsetLength;
//	int3 currentPosition = input.voxelPosition;

	float3 indirectColor = 0;

	//for (uint i = 0; i < sampleCount; ++i)
	//for (uint direction = 0; direction < 4; ++direction)
	float3 sampleDirection = input.normal;//mul(input.normal, rotationMatrices[direction]);

	float3 offset = sampleDirection * texelSize;
	float offsetLength = length(offset);

	float3 currentPosition = coords + offset * 2;

	float traveledDistance = 0.0;
	float luminance = 0.0;

	uint mipLevel = 0;
	
	float minDiameter = 1.0 / float(voxel.dim);
	float minVoxelDiameterInv = float(voxel.dim);
	const float coneRatio = 0.3;

	while (traveledDistance < maxDist && luminance < 1.0)
	{
		int3 currentVoxelPosition = currentPosition * int3(voxelDim);

		float sampleDiameter = max(minDiameter, coneRatio * traveledDistance);
		float sampleLOD = log2(sampleDiameter * minVoxelDiameterInv);
		mipLevel = uint(sampleLOD);
		mipLevel = clamp(mipLevel, 0, g_maxMipLevel);

		currentVoxelPosition.x = currentVoxelPosition.x >> mipLevel;
		currentVoxelPosition.y = currentVoxelPosition.y >> mipLevel;
		currentVoxelPosition.z = currentVoxelPosition.z >> mipLevel;

		uint packedColor = g_voxelTextureDiffuse.Load(int4(currentVoxelPosition, mipLevel)).r;
		float4 sampleColor = unpackColor(packedColor);
		float sampleLuminance = sampleColor.a;

		indirectColor += sampleColor.rgb;
		luminance += sampleLuminance;
		currentPosition += offset;

		traveledDistance += offsetLength;
	}

	return float4(indirectColor, 1.0f);
}