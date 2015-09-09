#include "GpuTransform.h"
#include "GpuMath.h"
#include "GpuShadowTransform.h"
#include "GpuCameraBufferData.h"

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
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint lightIndex : BLENDINDICES0;
	uint material : BLENDINDICES1;
	float distanceToLight : BLENDINDICES2;
	float lightFalloff : BLENDINDICES3;
	float lightLumen : BLENDINDICES4;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<ShadowTransform> shadowTransforms : register(t1);
StructuredBuffer<uint> s_materials : register(t2);

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

VSOutput main(Vertex input)
{
	const float3 lightPositionWS = float3(0, 2.8, -1);
	const float lightFalloff = 5.0;
	const float lightLumen = 100.0;

	uint elementId = input.drawId & 0xffff;
	uint materialIndex = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = unpackQRotation(s_transforms[elementId].packedQRotation);

	float3 wsPosition = quaternionRotation(input.position.xyz, qRotation) + translation;
	float3 wsNormal = quaternionRotation(input.normal, qRotation);
	float3 vsNormal = mul(cameraBuffer.viewMatrix, float4(wsNormal, 0)).xyz;

	float distanceToLight = length(lightPositionWS - wsPosition);// / lightFalloff;

	VSOutput output;
	output.wsPosition = wsPosition;
	output.vsNormal = vsNormal;
	output.texCoords = input.texCoords;
	output.lightIndex = 0;
	output.material = s_materials[materialIndex];
	output.distanceToLight = distanceToLight;
	output.lightFalloff = lightFalloff;
	output.lightLumen = lightLumen;
	return output;
}