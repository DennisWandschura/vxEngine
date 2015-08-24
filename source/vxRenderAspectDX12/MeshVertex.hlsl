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

struct GSInput
{
	float3 wsPosition : POSITION0;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<uint> s_materials : register(t1);

GSInput main(Vertex input)
{
	uint elementId = input.drawId & 0xffff;
	uint materialIndex = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = unpackQRotation(s_transforms[elementId].packedQRotation);

	float3 bitangent = cross(input.normal, input.tangent) * input.position.w;

	float3 wsPosition = quaternionRotation(input.position.xyz, qRotation) + translation;
	float3 wsNormal = quaternionRotation(input.normal, qRotation);
	float3 wsTangent = quaternionRotation(input.tangent, qRotation);
	float3 wsBitangent = quaternionRotation(bitangent, qRotation);

	float3x3 normalMatrix = (float3x3)cameraBuffer.viewMatrix;
	float3 vsNormal = mul(normalMatrix, wsNormal);

	GSInput output;
	output.wsPosition = wsPosition;
	output.vsNormal = vsNormal;
	output.texCoords = input.texCoords;
	output.material = s_materials[materialIndex];

	return output;
}