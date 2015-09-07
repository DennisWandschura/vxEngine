#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"
#include "GpuMath.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	uint2 texelCoord : BLENDINDICES0;
};

struct GSOutput
{
	float4 position : SV_POSITION;
	float3 normal : NORMAL0;
	int3 voxelPosition : POSITION0;
	uint rotationIndex : BLENDINDICES0;
	uint offsetZ : BLENDINDICES1;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

cbuffer CameraBuffer : register(b1)
{
	GpuCameraBufferData cameraBuffer;
};

cbuffer CameraStaticBuffer : register(b2)
{
	GpuCameraStatic cameraStatic;
};

Texture2D<float> g_zBuffer : register(t0);
Texture2DArray<float2> g_normalSlice : register(t1);

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraStatic.projInfo.xy + cameraStatic.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP, float z)
{
	float3 P;

	P.z = z * cameraStatic.zFar;

	P = reconstructCSPosition(float2(ssP), P.z);

	return P;
}

// 3 axis, 7 directions
[maxvertexcount(3)]
void main(
	point VSOutput input[1],
	inout PointStream< GSOutput > output
	)
{
	uint2 texelCoord = input[0].texelCoord;
	float z = g_zBuffer.Load(int3(texelCoord, 0)).r;
	float3 positionVS = getPosition(texelCoord, z);
	float3 wsPosition = mul(cameraBuffer.invViewMatrix, float4(positionVS, 1)).xyz;

	float3 voxelCenter = cameraBuffer.position.xyz * voxel.invGridCellSize;
	voxelCenter = float3(int3(voxelCenter)) * voxel.gridCellSize;

	float3 offset = (wsPosition - voxelCenter) * voxel.invGridCellSize;
	int3 voxelPosition = int3(offset)+voxel.halfDim;
	if (voxelPosition.x < 0 || voxelPosition.x >= voxel.dim ||
		voxelPosition.y < 0 || voxelPosition.y >= voxel.dim ||
		voxelPosition.z < 0 || voxelPosition.z >= voxel.dim)
		return;

	float2 packedNormal = g_normalSlice.Load(int4(texelCoord, 0, 0)).rg;
	float3 vsNormal = decodeNormal(packedNormal.rg);
	float3 wsNormal = mul(cameraBuffer.invViewMatrix, float4(vsNormal, 0)).xyz;
	wsNormal = normalize(wsNormal);

	uint signedAxis[3];
	signedAxis[0] = (wsNormal.x < 0) ? 0 : 1;
	signedAxis[1] = (wsNormal.y < 0) ? 2 : 3;
	signedAxis[2] = (wsNormal.z < 0) ? 4 : 5;

	uint voxelOffsetZ[3];
	voxelOffsetZ[0] = signedAxis[0] * voxel.dim;
	voxelOffsetZ[1] = signedAxis[1] * voxel.dim;
	voxelOffsetZ[2] = signedAxis[2] * voxel.dim;

	for (uint axis = 0; axis < 3; ++axis)
	{
		//uint axis = 0;
		uint3 currentVoxelPosition = voxelPosition;
		//emitPoint(input[0].position, wsNormal, currentVoxelPosition, signedAxis[axis], output);

		//for (uint direction = 0; direction < 7; ++direction)
		{
			GSOutput element;
			element.position = input[0].position;
			element.normal = wsNormal;
			element.voxelPosition = voxelPosition;
			element.offsetZ = voxelOffsetZ[axis];
			element.rotationIndex = 0;
			output.Append(element);
			output.RestartStrip();
		}
	}
}