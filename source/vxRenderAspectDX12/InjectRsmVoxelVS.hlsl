#include "GpuShadowTransform.h"
#include "gpuvoxel.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	int3 gridPosition : POSITION0;
	int axis : BLENDINDICES0;
	float3 color : COLOR0;
	float4 normal : NORMAL0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel g_voxel;
};

cbuffer RootBuffer : register(b1)
{
	uint g_lightIndex;
	uint g_cubeIndex;
};

Texture2DArray<float4> g_color : register(t0);
Texture2DArray<half4> g_normals : register(t1);
Texture2DArray<float> g_depth : register(t2);

StructuredBuffer<GpuShadowTransformReverse> g_shadowTransforms : register(t3);

// flip axis +-
static const uint g_voxelAxis[6] = {1, 0, 3, 2, 4, 5};

static const float3 g_swizzleVector[3]=
{
	float3(1, 0, 0),
	float3(0, 1, 0),
	float3(0, 0, 1)
};

bool isPointInGrid(int3 position)
{
	bool result = true;
	if (position.x < 0 || position.y < 0 || position.z < 0 ||
		position.x >= g_voxel.dim || position.y >= g_voxel.dim || position.z >= g_voxel.dim)
		result = false;

	return result;
}

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	int arrayIndex = g_lightIndex * 6 + g_cubeIndex;
	float depth = g_depth.Load(int4(vertexID, instanceID, arrayIndex, 0));

	int2 texelCoord = int2(vertexID, instanceID);

	uint w, h, d;
	g_depth.GetDimensions(w, h, d);
	float2 texCoord = float2(texelCoord) / float2(w, h);
	float2 screenPos = texCoord * float2(2, -2) - float2(1, -1);

	float4x4 invPvMatrix = g_shadowTransforms[g_lightIndex].invPvMatrix[g_cubeIndex];
	float4 wsPosition = mul(invPvMatrix, float4(screenPos, depth, 1));

	float3 offset = (wsPosition.xyz - g_voxel.gridCenter.xyz) * g_voxel.invGridCellSize;
	int3 gridPosition = int3(offset)+g_voxel.halfDim;

	float3 color = g_color.Load(int4(texelCoord, arrayIndex, 0)).rgb;
	float3 normal = g_normals.Load(int4(texelCoord, arrayIndex, 0)).rgb;

	VSOutput output;
	output.position = float4(screenPos, 0, 1);
	
	output.axis = g_voxelAxis[g_cubeIndex];
	output.color = color;
	output.normal = float4(normal, dot(g_swizzleVector[g_cubeIndex / 2], abs(normal)));

	if (!isPointInGrid(gridPosition))
	{
		output.position.xy = -2;
	}

	int3 wOffset = int3(0, 0, g_voxelAxis[g_cubeIndex] * g_voxel.dim);

	output.gridPosition = gridPosition + wOffset;

	return output;
}