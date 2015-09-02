#include "GpuTransform.h"
#include "GpuMath.h"

struct Vertex
{
	float4 position : POSITION0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float2 texCoords : TEXCOORD0;
	uint drawId : BLENDINDICES0;
};

struct VSOutput 
{
	float3 wsPosition : POSITION0;
	uint lightIndex : BLENDINDICES0;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);

VSOutput main(Vertex input)
{
	uint elementId = input.drawId & 0xffff;
	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = unpackQRotation(s_transforms[elementId].packedQRotation);

	float3 wsPosition = quaternionRotation(input.position.xyz, qRotation) + translation;

	VSOutput output;
	output.wsPosition = wsPosition;
	output.lightIndex = 0;
	return output;
}