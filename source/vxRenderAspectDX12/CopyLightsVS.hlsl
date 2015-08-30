#include "GpuLight.h"

StructuredBuffer<GpuLight> g_lights : register(t0);
StructuredBuffer<uint> g_visibleLights : register(t1);

RWByteAddressBuffer g_lightsDst : register(u0);

void main(uint index : SV_VertexID)
{
	uint srcIndex = index + 1;

	uint visible = g_visibleLights[srcIndex];

	if (visible != 0)
	{
		GpuLight light = g_lights[srcIndex];

		uint dstIndex;
		g_lightsDst.InterlockedAdd(0, 1, dstIndex);
		dstIndex = dstIndex + 1;

		uint dstAddress = dstIndex * 32;

		uint4 values[2];
		values[0].x = asuint(light.position.x);
		values[0].y = asuint(light.position.y);
		values[0].z = asuint(light.position.z);
		values[0].w = asuint(light.position.w);

		values[1].x = asuint(light.falloff);
		values[1].y = asuint(light.lumen);
		values[1].z = 0;
		values[1].w = 0;

		g_lightsDst.Store4(dstAddress, values[0]);
		dstAddress += 16;
		g_lightsDst.Store4(dstAddress, values[1]);
	}
}