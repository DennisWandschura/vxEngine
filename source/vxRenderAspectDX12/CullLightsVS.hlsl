#include "GpuLight.h"

struct VSOutput
{
	float4 positionFalloff : POSITION0;
	uint lightIndex : BLENDINDEX0;
};

StructuredBuffer<GpuLight> g_lights : register(t0);

VSOutput main(uint index : SV_VertexID)
{
	index = index + 1;

	VSOutput output;
	output.positionFalloff = float4(g_lights[index].position.xyz, g_lights[index].falloff);
	output.lightIndex = index;

	return output;
}