#include "GpuCameraBufferData.h"
#include "GpuTransform.h"
#include "GpuQuaternion.h"

struct Vertex
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint drawId : BLENDINDICES0;
};

struct PSIN
{
	float4 position : SV_POSITION;
	float3 wsPosition : POSITION1;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint materialId : BLENDINDICES0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<GpuTransform> s_transforms : register(t0);

PSIN main(Vertex input)
{
	uint elementId = input.drawId & 0xffff;
	uint materialId = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = s_transforms[elementId].qRotation;

	float3 wsPosition = quaternionRotation(input.position, qRotation) + translation;

	float3x3 normalMatrix = (float3x3)cameraBuffer.viewMatrix;
	float3 wsNormal = quaternionRotation(input.normal, qRotation);

	PSIN vsout;
	vsout.position = mul(cameraBuffer.pvMatrix, float4(wsPosition, 1));
	vsout.wsPosition = wsPosition;
	vsout.vsNormal = mul(normalMatrix, wsNormal);
	vsout.texCoords = input.texCoords;
	vsout.materialId = materialId;

	return vsout;
}