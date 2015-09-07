#include "GpuCameraBufferData.h"

struct Vertex
{
	float3 position : POSITION0;
	float2 texCoords : TEXCOORDS0;
	float3 color : COLOR0;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORDS0;
	float3 color : COLOR0;
};

cbuffer CameraStaticBuffer : register(b0)
{
	GpuCameraStatic cameraStatic;
};

VSOutput main(Vertex input)
{
	VSOutput output;

	output.position = mul(cameraStatic.orthoMatrix, float4(input.position, 1.0));
	output.color = input.color;
	output.texCoords = input.texCoords;

	return output;
}