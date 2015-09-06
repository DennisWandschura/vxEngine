#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"

struct VSOut
{
	uint3 voxelPos : BLENDINCIES0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 color : NORMAL0;
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

void outputQuad(float3 wsPosition[4], uint3 voxelPos, float3 color, inout TriangleStream< GSOutput > output)
{
	float3 cameraPosition = camera.position.xyz;
	//	cameraPosition.y = 0;
	float3 voxelCenter = cameraPosition * voxel.invGridCellSize;
	voxelCenter = float3(int3(voxelCenter)) * voxel.gridCellSize;

	wsPosition[0] = wsPosition[0] * voxel.gridCellSize + voxelCenter;
	wsPosition[1] = wsPosition[1] * voxel.gridCellSize + voxelCenter;
	wsPosition[2] = wsPosition[2] * voxel.gridCellSize + voxelCenter;
	wsPosition[3] = wsPosition[3] * voxel.gridCellSize + voxelCenter;

	GSOutput element;
	element.color = color;

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

[maxvertexcount(36)]
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

	uint3 textureOffset = uint3(0, 0, 0);
	float3 wsPosition[4];

	// +x
	uint valueX = g_voxelTexture.Load(int4(voxelPos, 0)).r;
	if (valueX != 0)
	{
		wsPosition[0] = float3(l, b, n);
		wsPosition[1] = float3(l, b, f);
		wsPosition[2] = float3(l, t, f);
		wsPosition[3] = float3(l, t, n);
		outputQuad(wsPosition, voxelPos, float3(0.25, 0.5, 0), output);
	}

	textureOffset.z = voxel.dim;
	uint valueXX = g_voxelTexture.Load(int4(voxelPos + textureOffset, 0)).r;
	if (valueXX != 0)
	{
		wsPosition[0] = float3(r, b, f);
		wsPosition[1] = float3(r, b, n);
		wsPosition[2] = float3(r, t, n);
		wsPosition[3] = float3(r, t, f);
		outputQuad(wsPosition, voxelPos, float3(0.25, 0.5, 0.25), output);
	}

	textureOffset.z = voxel.dim * 2;
	uint valueY = g_voxelTexture.Load(int4(voxelPos + textureOffset, 0)).r;
	if (valueY != 0)
	{
		wsPosition[0] = float3(l, b, n);
		wsPosition[1] = float3(r, b, n);
		wsPosition[2] = float3(r, b, f);
		wsPosition[3] = float3(l, b, f);
		outputQuad(wsPosition, voxelPos, float3(0.25, 0.1, 0.25), output);
	}

	textureOffset.z = 4 * voxel.dim;
	// +z
	uint value0 = g_voxelTexture.Load(int4(voxelPos + textureOffset, 0)).r;
	if (value0 != 0)
	{
		wsPosition[0] = float3(l, b, n);
		wsPosition[1] = float3(r, b, n);
		wsPosition[2] = float3(r, t, n);
		wsPosition[3] = float3(l, t, n);
		outputQuad(wsPosition, voxelPos, float3(0.5, 0, 0), output);
	}

	textureOffset.z += voxel.dim;
	// -z
	uint value1 = g_voxelTexture.Load(int4(voxelPos + textureOffset, 0)).r;
	if (value1 != 0)
	{
		wsPosition[0] = float3(r, b, f);
		wsPosition[1] = float3(l, b, f);
		wsPosition[2] = float3(l, t, f);
		wsPosition[3] = float3(r, t, f);
		outputQuad(wsPosition, voxelPos, float3(0.5, 0.5, 0), output);
	}
}