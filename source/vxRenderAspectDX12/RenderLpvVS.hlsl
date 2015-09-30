#include "gpuvoxel.h"
#include "GpuCameraBufferData.h"
#include "gpumath.h"

struct VSOutput
{
	float4 pos : SV_POSITION;
	uint2 texelPos : POSITION0;
	float3 fVoxelPosition : POSITION1;
	float3 wsPosition : POSITION2;
	float3 wsNormal : NORMAL0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel g_voxel;
};

cbuffer CameraBuffer : register(b1)
{
	GpuCameraBufferData g_camera;
};

cbuffer CameraStaticBuffer : register(b2)
{
	GpuCameraStatic g_cameraStatic;
};

Texture2D<float> g_zBuffer : register(t0);
Texture2DArray<half2> g_normalSlice : register(t1);

bool isPointInGrid(int3 position)
{
	bool result = true;
	if (position.x < 0 || position.y < 0 || position.z < 0 ||
		position.x >= g_voxel.dim || position.y >= g_voxel.dim || position.z >= g_voxel.dim)
		result = false;

	return result;
}

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * g_cameraStatic.projInfo.xy + g_cameraStatic.projInfo.zw) * z, -z);
}

float3 getVsPosition(int2 ssP)
{
	float3 P;

	P.z = g_zBuffer.Load(int3(ssP, 0)).r * g_cameraStatic.zFar;

	P = reconstructCSPosition(float2(ssP), P.z);

	return P;
}

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint w, h;
	g_zBuffer.GetDimensions(w, h);
	float2 resolution = float2(w, h);

	uint2 texelPos = uint2(vertexID, instanceID) * 4;
	float2 texCoord = texelPos / resolution;

	float3 vsPosition = getVsPosition(texelPos);
	float3 wsPosition = mul(g_camera.invViewMatrix, float4(vsPosition, 1)).xyz;

	float3 vsNormal = decodeNormal(g_normalSlice.Load(int4(texelPos, 0, 0)));
	vsNormal = normalize(vsNormal);
	float3 wsNormal = mul(g_camera.invViewMatrix, float4(vsNormal, 0)).xyz;

	float3 offset = (wsPosition.xyz - g_voxel.gridCenter.xyz) * g_voxel.invGridCellSize;
	float3 fVoxelPosition = offset + g_voxel.halfDim;
	int3 voxelPosition = int3(offset) + g_voxel.halfDim;

	float2 screenPos = texCoord * float2(2, -2) - float2(1, -1);

	VSOutput output;
	output.pos = float4(screenPos, 0, 1);
	output.texelPos = texelPos;
	output.wsPosition = wsPosition;
	output.fVoxelPosition = fVoxelPosition;
	output.wsNormal = wsNormal;

	if (!isPointInGrid(voxelPosition))
	{
		output.pos.xy = -2;
	}

	return output;
}