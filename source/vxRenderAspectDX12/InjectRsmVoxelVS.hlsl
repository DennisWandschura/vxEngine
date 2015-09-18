#include "GpuShadowTransform.h"
#include "gpuvoxel.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	int3 gridPosition : POSITION0;
	int axis : BLENDINDICES0;
	float3 color : COLOR0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel g_voxel;
};

cbuffer RootBuffer : register(b1)
{
	uint lightIndex;
	uint cubeIndex;
};

Texture2DArray<float4> g_color : register(t0);
Texture2DArray<half4> g_normals : register(t1);
Texture2DArray<float> g_depth : register(t2);

StructuredBuffer<GpuShadowTransformReverse> g_transforms : register(t3);

// flip axis +-
static const uint g_voxelAxis[6] = {1, 0, 3, 2, 5, 4};

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
	int arrayIndex = lightIndex * 6 + cubeIndex;
	float depth = g_depth.Load(int4(vertexID, instanceID, arrayIndex, 0));

	uint w, h, d;
	g_depth.GetDimensions(w, h, d);
	float2 texCoord = float2(vertexID, instanceID) / float2(w, h);
	float2 screenPos = float2(texCoord.x, texCoord.y) * float2(0.5, -0.5) + float2(0.5, -0.5);

	float4x4 invPvMatrix = g_transforms[lightIndex].invPvMatrix[cubeIndex];
	float4 wsPosition = mul(invPvMatrix, float4(screenPos, depth, 1));

	float3 offset = (wsPosition.xyz - g_voxel.gridCenter.xyz) * g_voxel.invGridCellSize;
	int3 gridPosition = int3(offset)+g_voxel.halfDim;

	float3 color = g_color.Load(int4(vertexID, instanceID, arrayIndex, 0)).rgb;

	VSOutput output;
	output.position = float4(0, 0, 0, 1);
	output.gridPosition = gridPosition;
	output.axis = g_voxelAxis[cubeIndex];
	output.color = color;

	if (!isPointInGrid(gridPosition))
	{
		output.position.xy = -2;
	}

	return output;
}