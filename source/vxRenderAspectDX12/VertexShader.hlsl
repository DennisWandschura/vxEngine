#include "CameraBufferData.h"

struct Vertex
{
	float3 position : POSITION0;
	float2 texCoords : TEXCOORD0;
	uint drawId : BLENDINDICES0;
};

struct PSIN
{
	float4 position : SV_POSITION;
	float3 color : COLOR0;
};

struct Transform
{
	float4 translation;
};

cbuffer CameraBuffer : register(b0)
{
	CameraBufferData cameraBuffer;
};

StructuredBuffer<Transform> s_transforms : register(t0);

PSIN main(Vertex input)
{
	uint elementId = input.drawId & 0xffff;
	uint materialId = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;

	float4 wsPosition = float4(input.position + translation, 1); //

	float ccc = float(elementId) / 128.0;

	PSIN vsout;
	vsout.position = mul(cameraBuffer.pvMatrix, wsPosition);
	vsout.color = float3(input.texCoords, ccc);
	//vsout.position = wsPosition;

	return vsout;
}