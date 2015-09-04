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
	uint lightIndex : BLENDINDICES0;
	float distanceToLight : BLENDINDICES1;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<ShadowTransform> shadowTransforms : register(t1);

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

VSOutput main(Vertex input)
{
	const float3 lightPositionWS = float3(0, 2.8, -1);
	const float lightFalloff = 5.0;

	uint elementId = input.drawId & 0xffff;
	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = unpackQRotation(s_transforms[elementId].packedQRotation);

	float3 wsPosition = quaternionRotation(input.position.xyz, qRotation) + translation;

	float distanceToLight = length(lightPositionWS - wsPosition) / lightFalloff;
	distanceToLight = clamp(distanceToLight, 0, 1);

	VSOutput output;
	output.wsPosition = wsPosition;
	output.lightIndex = 0;
	output.distanceToLight = distanceToLight;
	return output;
}