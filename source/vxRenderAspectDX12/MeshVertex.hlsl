#include "GpuCameraBufferData.h"
#include "GpuTransform.h"
#include "GpuQuaternion.h"

struct Vertex
{
	float3 position : POSITION0;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float2 uv : TEXCOORD0;
	uint drawId : BLENDINDICES0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float3 vsPosition : POSITION1;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float2 uv : TEXCOORD0;
	uint materialId : BLENDINDICES0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<GpuTransform> s_transforms : register(t0);

float4 main(Vertex input) : SV_POSITION
{
	uint elementId = input.drawId & 0xffff;
	uint materialId = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = s_transforms[elementId].qRotation;

	float3 wsPosition = quaternionRotation(input.position, qRotation) + translation;
	float3 wsNormal = quaternionRotation(input.normal, qRotation);

	float3x3 normalMatrix = (float3x3)cameraBuffer.viewMatrix;
	//float3 vsPosition = mul(float3x3(cameraBuffer.viewMatrix), wsPosition.xyz);

	PSInput output;
	output.position = mul(cameraBuffer.pvMatrix, float4(wsPosition, 1));

	return float4(input.position, 1);
}