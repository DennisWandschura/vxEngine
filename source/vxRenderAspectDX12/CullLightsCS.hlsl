#include "GpuLight.h"

cbuffer FrustumBuffer : register(b0)
{
	float4 nd[3];
};

StructuredBuffer<GpuLight> g_lights : register(t0);
RWByteAddressBuffer g_lightsDst : register(u0);

bool testFrustumPlane(in float3 c, float r, in float3 pn, float pd)
{
	float dist = dot(c, pn);
	dist = dist - pd;
	dist = dist + r;

	return (dist >= 0);
}

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint lightIndex = DTid.x;

	GpuLight light = g_lights[lightIndex];

	bool inFrustum[3];

	float3 lightPosition = light.position.xyz;
	float falloff = light.falloff;
	[unroll]
	for (uint i = 0; i < 3; ++i)
	{
		float3 pn = nd[i].xyz;
		float pd = nd[i].w;

		inFrustum[i] = testFrustumPlane(lightPosition, falloff, pn, pd);
	}

	bool result = inFrustum[0] && inFrustum[1] && inFrustum[2];
	if (result)
	{
		uint dstIndex;
		g_lightsDst.InterlockedAdd(0, 1, dstIndex);
		dstIndex = dstIndex + 1;

		uint4 values[2];
		values[0].x = asuint(light.position.x);
		values[0].y = asuint(light.position.y);
		values[0].z = asuint(light.position.z);
		values[0].w = asuint(light.position.w);

		values[1].x = asuint(light.falloff);
		values[1].y = asuint(light.lumen);
		values[1].z = 0;
		values[1].w = 0;

		uint addressInBytes = dstIndex * 32;
		g_lightsDst.Store4(addressInBytes, values[0]);
		addressInBytes += 16;
		g_lightsDst.Store4(addressInBytes, values[1]);
	}
}