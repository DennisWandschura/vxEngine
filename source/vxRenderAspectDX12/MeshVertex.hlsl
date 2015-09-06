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
	float4 position : SV_POSITION;
	float4 positionPrev : POSITION1;
	float3 vsNormal : NORMAL0;
	float3 vsTangent : NORMAL1;
	float3 vsBitangent : NORMAL2;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<uint> s_materials : register(t1);
StructuredBuffer<TransformGpu> s_transformsPrev : register(t2);

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

	float3 vsNormal = mul(cameraBuffer.viewMatrix, float4(wsNormal, 0)).xyz;
	float3 vsTangent = mul(cameraBuffer.viewMatrix, float4(wsTangent, 0)).xyz;
	float3 vsBitangent = mul(cameraBuffer.viewMatrix, float4(wsBitangent, 0)).xyz;

	float3 translationPrev = s_transformsPrev[elementId].translation.xyz;
	float4 qRotationPrev = unpackQRotation(s_transformsPrev[elementId].packedQRotation);
	float3 wsPositionPrev = quaternionRotation(input.position.xyz, qRotationPrev) + translationPrev;

	GSInput output;
	output.position = mul(cameraBuffer.pvMatrix, float4(wsPosition, 1));
	output.positionPrev = mul(cameraBuffer.pvMatrixPrev, float4(wsPositionPrev, 1));
	output.vsNormal = vsNormal;
	output.vsTangent = vsTangent;
	output.vsBitangent = vsBitangent;
	output.texCoords = input.texCoords;
	output.material = s_materials[materialIndex];

	return output;
}