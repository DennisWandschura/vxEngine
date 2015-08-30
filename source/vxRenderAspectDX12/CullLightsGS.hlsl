#include "GpuCameraBufferData.h"

struct VSOutput
{
	float4 positionFalloff : POSITION0;
	uint lightIndex : BLENDINDEX0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	uint lightIndex : BLENDINDEX0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

void drawQuad(GSOutput element, uint i0, uint i1, uint i2, uint i3, inout TriangleStream< GSOutput > output, in float4 corners[8])
{
	element.pos = corners[i0];
	output.Append(element);

	element.pos = corners[i1];
	output.Append(element);

	element.pos = corners[i2];
	output.Append(element);

	output.RestartStrip();

	element.pos = corners[i0];
	output.Append(element);

	element.pos = corners[i2];
	output.Append(element);

	element.pos = corners[i3];
	output.Append(element);
}

[maxvertexcount(36)]
void main(
	point VSOutput input[1],
	inout TriangleStream< GSOutput > output
)
{
	float3 p = input[0].positionFalloff.xyz;
	float r = input[0].positionFalloff.w;

	float3 vmin = p - r;
	float3 vmax = p + r;

	float4 corners[8];
	// lbf
	corners[0] = mul(cameraBuffer.pvMatrix, float4(vmin, 1));
	// rbf
	corners[1] = mul(cameraBuffer.pvMatrix, float4(vmax.x, vmin.yz, 1));
	// lbn
	corners[2] = mul(cameraBuffer.pvMatrix, float4(vmin.xy, vmax.z, 1));
	// rbn
	corners[3] = mul(cameraBuffer.pvMatrix, float4(vmax.x, vmin.y, vmax.z, 1));

	// ltf
	corners[4] = mul(cameraBuffer.pvMatrix, float4(vmin.x, vmax.y, vmin.z, 1));
	// rtf
	corners[5] = mul(cameraBuffer.pvMatrix, float4(vmax.x, vmax.y, vmin.z, 1));
	// rtn
	corners[6] = mul(cameraBuffer.pvMatrix, float4(vmax.x, vmax.y, vmax.z, 1));
	// ltn
	corners[7] = mul(cameraBuffer.pvMatrix, float4(vmin.x, vmax.y, vmax.z, 1));

	GSOutput element;
	element.lightIndex = input[0].lightIndex;

	// bottom
	drawQuad(element, 2, 3, 1, 0, output, corners);
	output.RestartStrip();

	// top
	drawQuad(element, 7, 6, 5, 4, output, corners);
	output.RestartStrip();

	// front
	drawQuad(element, 2, 3, 6, 7, output, corners);
	output.RestartStrip();

	// back
	drawQuad(element, 0, 1, 5, 4, output, corners);
	output.RestartStrip();

	// left
	drawQuad(element, 0, 2, 7, 4, output, corners);
	output.RestartStrip();

	// right
	drawQuad(element, 1, 3, 6, 5, output, corners);
	output.RestartStrip();
}