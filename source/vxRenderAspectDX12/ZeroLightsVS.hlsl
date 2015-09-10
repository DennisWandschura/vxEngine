#include "GpuLight.h"

RWStructuredBuffer<uint> g_visibleLights : register(u0);

void main(uint index : SV_VertexID)
{
	g_visibleLights[index] = 0;
}