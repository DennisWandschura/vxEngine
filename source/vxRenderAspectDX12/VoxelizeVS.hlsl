#include "GpuCameraBufferData.h"
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

struct VSOut
{
	float3 wsPosition : POSITION0;
	uint material : BLENDINDICES0;
	float2 texCoords : TEXCOORD0;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<uint> s_materials : register(t1);

VSOut main(Vertex input)
{
	uint elementId = input.drawId & 0xffff;
	uint materialIndex = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = unpackQRotation(s_transforms[elementId].packedQRotation);
	float3 wsPosition = quaternionRotation(input.position.xyz, qRotation) + translation;

	VSOut output;
	output.wsPosition = wsPosition;
	output.material = s_materials[materialIndex];;
	output.texCoords = input.texCoords;

	return output;
}