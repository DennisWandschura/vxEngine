#include "GpuCameraBufferData.h"
#include "GpuLight.h"

struct VSOutput
{
	float3 wsPosition : POSITION0;
	float2 falloffLumen : BLENDINDICES0;
	uint lightIndex : BLENDINDICES1;
};

StructuredBuffer<GpuLight> g_lights : register(t0);

VSOutput main(uint lightIndex : SV_VertexID)
{
	float3 lightPosition = g_lights[lightIndex].position.xyz;
	float lightFalloff = g_lights[lightIndex].falloff;
	float lightLumen = g_lights[lightIndex].lumen;

	VSOutput output;
	output.wsPosition = lightPosition;
	output.falloffLumen = float2(lightFalloff, lightLumen);
	output.lightIndex = lightIndex;

	return output;
}