#include "GpuTransform.h"
#include "GpuMath.h"
#include "gpulight.h"
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
	float3 lightPositionWS : POSITION1;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint lightIndex : BLENDINDICES0;
	uint material : BLENDINDICES1;
	float distanceToLight : BLENDINDICES2;
	float lightFalloff : BLENDINDICES3;
	float lightLumen : BLENDINDICES4;
};

StructuredBuffer<TransformGpu> s_transforms : register(t0);
StructuredBuffer<uint> s_materials : register(t1);
StructuredBuffer<GpuLight> g_shadowLights : register(t2);

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

cbuffer RootSignatureConstants : register(b1)
{
	uint g_lightIndex;
};

VSOutput main(Vertex input)
{
	uint lightIndex = g_lightIndex;

	float3 lightPositionWS = g_shadowLights[lightIndex].position.xyz;
	const float lightFalloff = g_shadowLights[lightIndex].falloff;
	const float lightLumen = g_shadowLights[lightIndex].lumen;

	uint elementId = input.drawId & 0xffff;
	uint materialIndex = input.drawId >> 16;

	float3 translation = s_transforms[elementId].translation.xyz;
	float4 qRotation = unpackQRotation(s_transforms[elementId].packedQRotation);

	float3 wsPosition = quaternionRotation(input.position.xyz, qRotation) + translation;
	float3 wsNormal = quaternionRotation(input.normal, qRotation);

	float distanceToLight = length(lightPositionWS - wsPosition);

	VSOutput output;
	output.wsPosition = wsPosition;
	output.lightPositionWS = lightPositionWS;
	output.normal = wsNormal;
	output.texCoords = input.texCoords;
	output.lightIndex = lightIndex;
	output.material = s_materials[materialIndex];
	output.distanceToLight = distanceToLight;
	output.lightFalloff = lightFalloff;
	output.lightLumen = lightLumen;
	return output;
}