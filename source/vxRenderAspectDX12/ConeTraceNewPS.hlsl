#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"

struct GSOutput
{
	float4 position : SV_POSITION;
};

static const uint g_maxMipLevel = 0;

Texture3D<uint> g_voxelTextureDiffuse : register(t2);

cbuffer CameraStaticBuffer : register(b2)
{
	GpuCameraStatic cameraStatic;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

cbuffer CameraBuffer : register(b1)
{
	GpuCameraBufferData cameraBuffer;
};

Texture2D<float> g_zBuffer : register(t0);

float4 unpackColor(uint packedColor)
{
	uint4 color = uint4(packedColor, packedColor >> 8, packedColor >> 16, packedColor >> 24) & 0xff;

	float4 unpackedColor = float4(color) / 255.0;
	return unpackedColor;
}

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraStatic.projInfo.xy + cameraStatic.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP)
{
	float z = g_zBuffer.Load(int3(ssP, 0)).r * cameraStatic.zFar;

	return reconstructCSPosition(float2(ssP), z);
}

float4 main(GSOutput input) : SV_TARGET
{
	float3 positionVS = getPosition(input.position.xy);
	float3 wsPosition = mul(cameraBuffer.invViewMatrix, float4(positionVS, 1)).xyz;

	float3 voxelCenter = cameraBuffer.position.xyz * voxel.invGridCellSize;
	voxelCenter = float3(int3(voxelCenter)) * voxel.gridCellSize;

	float3 offset = (wsPosition - voxelCenter) * voxel.invGridCellSize;
	int3 voxelPosition = int3(offset)+voxel.halfDim;

	if (voxelPosition.x < 0 || voxelPosition.x >= voxel.dim ||
		voxelPosition.y < 0 || voxelPosition.y >= voxel.dim ||
		voxelPosition.z < 0 || voxelPosition.z >= voxel.dim)
		discard;

	uint3 voxelDim = uint3(voxel.dim, voxel.dim, voxel.dim * 6);

	float3 texelSize = 1.0 / float3(voxelDim);


	float3 coords = voxelPosition * texelSize;

	float3 indirectColor = 0;

	/*float3 sampleDirection = input.normal;

	float3 offset = sampleDirection * texelSize;
	float offsetLength = length(offset);

	float3 currentPosition = coords + offset;

	float traveledDistance = 0.0;
	float luminance = 0.0;
	while (traveledDistance < 0.2 && luminance < 1.0)
	{
		int3 currentVoxelPosition = currentPosition * int3(voxelDim);
		uint packedColor = g_voxelTextureDiffuse.Load(int4(currentVoxelPosition, 0)).r;
		float4 sampleColor = unpackColor(packedColor);
		float sampleLuminance = sampleColor.a;

		indirectColor += sampleColor.rgb;
		luminance += sampleLuminance;
		currentPosition += offset;

		traveledDistance += offsetLength;
	}*/

	return float4(indirectColor, 1.0f);
}