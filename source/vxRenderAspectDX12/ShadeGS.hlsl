#include "GpuCameraBufferData.h"

struct VSOutput
{
	float3 wsPosition : POSITION0;
	float3 vsPosition : POSITION1;
	float2 falloffLumen : BLENDINDICES0;
	uint lightIndex : BLENDINDICES1;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	float3 vsPosition : POSITION1;
	float2 falloffLumen : BLENDINDICES0;
	uint lightIndex : BLENDINDICES1;
};

static const float2 g_positions[4] =
{
	{ -1, -1 },{ 1, -1 },{ 1, 1 },{ -1, 1 }
};

static const float2 g_texCoords[4] =
{
	{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 }
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData camera;
};

void emitQuad(GSOutput element, in float4 positions[4], inout TriangleStream< GSOutput > output)
{
	element.pos = positions[0];
	output.Append(element);

	element.pos = positions[1];
	output.Append(element);

	element.pos = positions[2];
	output.Append(element);

	output.RestartStrip();

	element.pos = positions[0];
	output.Append(element);

	element.pos = positions[2];
	output.Append(element);

	element.pos = positions[3];
	output.Append(element);

	output.RestartStrip();
}

[maxvertexcount(6 * 6)]
void main(
	point VSOutput input[1],
	inout TriangleStream< GSOutput > output
	)
{
	float3 tmpPos = input[0].wsPosition.xyz;

	float radius = input[0].falloffLumen.x;

	float3 vmin = tmpPos - radius;
	float3 vmax = tmpPos + radius;

	float l = vmin.x;
	float r = vmax.x;
	float b = vmin.y;
	float t = vmax.y;
	float n = vmax.z;
	float f = vmin.z;

	GSOutput element;
	element.wsPosition = input[0].wsPosition;
	element.vsPosition = input[0].vsPosition;
	element.falloffLumen = input[0].falloffLumen;
	element.lightIndex = input[0].lightIndex;

	float4 positions[4];
	// front
	positions[0] = mul(camera.pvMatrix, float4(l, b, f, 1));
	positions[1] = mul(camera.pvMatrix, float4(r, b, f, 1));
	positions[2] = mul(camera.pvMatrix, float4(r, t, f, 1));
	positions[3] = mul(camera.pvMatrix, float4(l, t, f, 1));
	emitQuad(element, positions, output);

	// back
	positions[0] = mul(camera.pvMatrix, float4(r, b, n, 1));
	positions[1] = mul(camera.pvMatrix, float4(l, b, n, 1));
	positions[2] = mul(camera.pvMatrix, float4(l, t, n, 1));
	positions[3] = mul(camera.pvMatrix, float4(r, t, n, 1));
	emitQuad(element, positions, output);

	// left
	positions[0] = mul(camera.pvMatrix, float4(l, b, n, 1));
	positions[1] = mul(camera.pvMatrix, float4(l, b, f, 1));
	positions[2] = mul(camera.pvMatrix, float4(l, t, f, 1));
	positions[3] = mul(camera.pvMatrix, float4(l, t, n, 1));
	emitQuad(element, positions, output);

	// right
	positions[0] = mul(camera.pvMatrix, float4(r, b, f, 1));
	positions[1] = mul(camera.pvMatrix, float4(r, b, n, 1));
	positions[2] = mul(camera.pvMatrix, float4(r, t, n, 1));
	positions[3] = mul(camera.pvMatrix, float4(r, t, f, 1));
	emitQuad(element, positions, output);

	// bottom
	positions[0] = mul(camera.pvMatrix, float4(l, b, n, 1));
	positions[1] = mul(camera.pvMatrix, float4(r, b, n, 1));
	positions[2] = mul(camera.pvMatrix, float4(r, b, f, 1));
	positions[3] = mul(camera.pvMatrix, float4(l, b, f, 1));
	emitQuad(element, positions, output);

	// top
	positions[0] = mul(camera.pvMatrix, float4(l, t, f, 1));
	positions[1] = mul(camera.pvMatrix, float4(r, t, f, 1));
	positions[2] = mul(camera.pvMatrix, float4(r, t, n, 1));
	positions[3] = mul(camera.pvMatrix, float4(l, t, n, 1));
	emitQuad(element, positions, output);
}