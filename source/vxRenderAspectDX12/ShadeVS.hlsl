#include "GpuCameraBufferData.h"
#include "GpuLight.h"

struct VSOutput
{
	float3 wsPosition : POSITION0;
	float3 vsPosition : POSITION1;
	float2 falloffLumen : BLENDINDICES0;
	uint lightIndex : BLENDINDICES1;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<GpuLight> g_lights : register(t0);

VSOutput main(uint vertexID : SV_VertexID)
{
	uint lightIndex = vertexID;

	float3 lightPosition = g_lights[lightIndex].position.xyz;
	float lightFalloff = g_lights[lightIndex].falloff;
	float lightLumen = g_lights[lightIndex].lumen;

	float3 lightPositionVS = mul(cameraBuffer.viewMatrix, float4(lightPosition, 1)).xyz;

	VSOutput output;
	output.wsPosition = lightPosition;
	output.vsPosition = lightPositionVS;
	output.falloffLumen = float2(lightFalloff, lightLumen);
	output.lightIndex = lightIndex;

	return output;
}