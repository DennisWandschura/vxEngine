#include "GpuLight.h"

RWStructuredBuffer<uint> g_visibleLights : register(u0);
RWStructuredBuffer<GpuLight> g_lightsDst : register(u1);

void main(uint index : SV_VertexID)
{
	g_visibleLights[index] = 0;
	g_lightsDst[0].position.x = 0;
}