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
	//float4 positionPrev : POSITION1;
	//float3 vsPosition : POSITION2;
	float3 vsNormal : NORMAL0;
	//float3 vsTangent : TANGENT0;
	//float3 vsBitangent : BITANGENT0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<uint> s_materials : register(t1);

/*float3 getPreviousWsPosition(in float3 position, uint index)
{
	float3 translation = s_transformsPrev[index].translation.xyz;
	float4 qRotation = unpackQRotation(s_transformsPrev[index].packedQRotation);

	float3 wsPosition = quaternionRotation(position, qRotation) + translation;

	return wsPosition;
}*/

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
	float3 vsTangent = mul(normalMatrix, wsTangent);
	float3 vsBitangent = mul(normalMatrix, wsBitangent);

	//float3 wsPositionPrev = getPreviousWsPosition(input.position, elementId);
	//s_transformsPrev[elementId] = s_transforms[elementId];

	GSInput output;
	output.wsPosition = wsPosition;
	//output.position = mul(cameraBuffer.pvMatrix, float4(wsPosition, 1));
	//output.vsPosition = mul(cameraBuffer.viewMatrix, float4(wsPosition, 1)).xyz;
	//output.positionPrev = output.position;//mul(cameraBuffer.pvMatrixPrev, float4(wsPositionPrev, 1));
	output.vsNormal = vsNormal;
	//output.vsTangent = vsTangent;
	//output.vsBitangent = vsBitangent;
	output.texCoords = input.texCoords;
	output.material = s_materials[materialIndex];

	return output;
}