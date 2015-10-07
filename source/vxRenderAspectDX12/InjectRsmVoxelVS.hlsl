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

VSOutput main(uint xy : SV_VertexID, uint cubeIndex : SV_InstanceID)
{
	uint w, h, d;
	g_depth.GetDimensions(w, h, d);
	uint x = xy & (w - 1);
	uint y = xy / w;
	int2 texelCoord = int2(x, y);

	int arrayIndex = g_lightIndex * 6 + cubeIndex;
	float depth = g_depth.Load(int4(texelCoord, arrayIndex, 0));

	float2 texCoord = float2(texelCoord) / float2(w, h);
	float2 screenPos = texCoord * float2(2, -2) - float2(1, -1);

	float3 normal = g_normals.Load(int4(texelCoord, arrayIndex, 0)).rgb;

	float4x4 invPvMatrix = g_shadowTransforms[g_lightIndex].invPvMatrix[cubeIndex];
	float4 wsPosition = mul(invPvMatrix, float4(screenPos, depth, 1));

	// shift gridposition by half cell size
	wsPosition.xyz += g_voxel.gridCellSize * 0.5 * normal;

	float3 offset = (wsPosition.xyz - g_voxel.gridCenter.xyz) * g_voxel.invGridCellSize;
	int3 gridPosition = int3(offset) + g_voxel.halfDim;

	float3 color = g_color.Load(int4(texelCoord, arrayIndex, 0)).rgb;

	VSOutput output;
	output.position = float4(screenPos, 0, 1);
	
	output.axis = g_voxelAxis[cubeIndex];
	output.color = color;

	float tmp = abs(dot(g_swizzleVector[cubeIndex / 2], normal));
	output.normal = float4(normal * 0.5 + 0.5, tmp);

	if (!isPointInGrid(gridPosition))
	{
		output.position.xy = -2;
	}

	int3 wOffset = int3(0, 0, g_voxelAxis[cubeIndex] * g_voxel.dim);

	output.gridPosition = gridPosition + wOffset;

	return output;
}