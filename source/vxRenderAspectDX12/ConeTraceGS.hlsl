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
	uint3 voxelPosition : POSITION0;
	uint rotationIndex : BLENDINDICES0;
	uint signedAxis : BLENDINDICES1;
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
Texture2DArray<float2> g_normalSlice : register(t1);

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraBuffer.projInfo.xy + cameraBuffer.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP, float z)
{
	float3 P;

	P.z = z * cameraBuffer.zFar;

	P = reconstructCSPosition(float2(ssP), P.z);

	return P;
}

void emitPoint(in float4 position, in float3 normal, in uint3 voxelPosition, uint signedAxis, inout PointStream< GSOutput > output)
{
	GSOutput element;
	element.position = position;
	element.normal = normal;
	element.voxelPosition = voxelPosition;
	element.signedAxis = signedAxis;

	//for (uint direction = 0; direction < 7; ++direction)
	{
		uint direction = 0;
		element.rotationIndex = direction;
		output.Append(element);
		output.RestartStrip();
	}
}

// 3 axis, 7 directions
[maxvertexcount(3 * 7)]
void main(
	point VSOutput input[1],
	inout PointStream< GSOutput > output
)
{
	uint2 texelCoord = input[0].texelCoord;
	float z = g_zBuffer.Load(int3(texelCoord, 0)).r;
	float3 positionVS = getPosition(texelCoord, z);
	float3 wsPosition = mul(cameraBuffer.invViewMatrix, float4(positionVS, 1)).xyz;

	float2 packedNormal = g_normalSlice.Load(int4(texelCoord, 0, 0)).rg;
	float3 vsNormal = decodeNormal(packedNormal.rg);
	float3 wsNormal = mul(cameraBuffer.invViewMatrix, float4(vsNormal, 0)).xyz;

	float3 voxelCenter = cameraBuffer.position.xyz * voxel.invGridCellSize;
	voxelCenter = float3(int3(voxelCenter)) * voxel.gridCellSize;

	float3 offset = (wsPosition - voxelCenter) * voxel.invGridCellSize;
	uint3 voxelPosition = uint3(offset) + voxel.halfDim;
	if (voxelPosition.x < 0 || voxelPosition.x >= voxel.dim ||
		voxelPosition.y < 0 || voxelPosition.y >= voxel.dim ||
		voxelPosition.z < 0 || voxelPosition.z >= voxel.dim)
		return;

	uint signedAxis[3];
	signedAxis[0] = (wsNormal.x < 0) ? 0 : 1;
	signedAxis[1] = (wsNormal.y < 0) ? 2 : 3;
	signedAxis[2] = (wsNormal.z < 0) ? 4 : 5;

	uint voxelPositionZ[3];
	voxelPositionZ[0] = signedAxis[0] * voxel.dim;
	voxelPositionZ[1] = signedAxis[1] * voxel.dim;
	voxelPositionZ[2] = signedAxis[2] * voxel.dim;

	//for (uint axis = 0; axis < 3; ++axis)
	{
		uint axis = 0;
		uint3 currentVoxelPosition = uint3(voxelPosition.xy, voxelPosition.z + voxelPositionZ[axis]);
		emitPoint(input[0].position, wsNormal, currentVoxelPosition, signedAxis[axis], output);
	}
}