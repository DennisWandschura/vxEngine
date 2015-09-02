#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"

struct VSOut
{
	uint3 voxelPos : BLENDINCIES0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

cbuffer CameraBuffer : register(b1)
{
	GpuCameraBufferData camera;
};

Texture3D<uint> g_voxelTexture : register(t0);

void outputQuad(in float3 wsPosition[4], uint3 voxelPos, inout TriangleStream< GSOutput > output)
{
	wsPosition[0] = wsPosition[0] * voxel.gridCellSize;
	wsPosition[1] = wsPosition[1] * voxel.gridCellSize;
	wsPosition[2] = wsPosition[2] * voxel.gridCellSize;
	wsPosition[3] = wsPosition[3] * voxel.gridCellSize;

	GSOutput element;
	element.pos = mul(camera.pvMatrix, float4(wsPosition[0], 1));
	output.Append(element);

	element.pos = mul(camera.pvMatrix, float4(wsPosition[1], 1));
	output.Append(element);

	element.pos = mul(camera.pvMatrix, float4(wsPosition[2], 1));
	output.Append(element);

	output.RestartStrip();

	element.pos = mul(camera.pvMatrix, float4(wsPosition[0], 1));
	output.Append(element);

	element.pos = mul(camera.pvMatrix, float4(wsPosition[2], 1));
	output.Append(element);

	element.pos = mul(camera.pvMatrix, float4(wsPosition[3], 1));
	output.Append(element);

	output.RestartStrip();
}

[maxvertexcount(12)]
void main(
	point VSOut input[1],
	inout TriangleStream< GSOutput > output
)
{
	uint3 voxelPos = input[0].voxelPos;

	float3 offset = voxel.halfDim;

	float l = voxelPos.x - offset.x;
	float r = voxelPos.x + 1 - offset.x;

	float b = voxelPos.y - offset.y;
	float t = voxelPos.y + 1 - offset.y;

	float f = voxelPos.z - offset.z;
	float n = voxelPos.z + 1 - offset.z;

	// +z
	uint value0 = g_voxelTexture.Load(int4(voxelPos, 0)).r;
	if (value0 != 0)
	{
		float3 wsPosition[4];
		wsPosition[0] = float3(l, b, n);
		wsPosition[1] = float3(r, b, n);
		wsPosition[2] = float3(r, t, n);
		wsPosition[3] = float3(l, t, n);
		outputQuad(wsPosition, voxelPos, output);
	}

	// -z
	uint value1 = g_voxelTexture.Load(int4(voxelPos.x, voxelPos.y, voxelPos.z + voxel.dim, 0)).r;
	if (value1 != 0)
	{
		float3 wsPosition[4];
		wsPosition[0] = float3(r, b, f);
		wsPosition[1] = float3(l, b, f);
		wsPosition[2] = float3(l, t, f);
		wsPosition[3] = float3(r, t, f);

		outputQuad(wsPosition, voxelPos, output);
	}
}