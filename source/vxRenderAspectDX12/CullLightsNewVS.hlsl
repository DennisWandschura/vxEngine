#include "GpuLight.h"
#include "GpuCameraBufferData.h"

Texture2D<float> g_zBuffer : register(t0);
StructuredBuffer<GpuLight> g_lights : register(t1);

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

cbuffer CameraStaticBuffer : register(b1)
{
	GpuCameraStatic cameraStatic;
};

cbuffer RootBuffer : register(b2)
{
	uint g_lightCount;
};

RWStructuredBuffer<uint> g_visibleLights : register(u0);

uint testLight(uint index, in float3 wsPosition)
{
	float3 position = g_lights[index].position.xyz;
	float radius = g_lights[index].falloff;

	float3 v = wsPosition - position;
	float distance2 = dot(v, v);

	return (distance2 <= radius * radius);
}

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraStatic.projInfo.xy + cameraStatic.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP)
{
	float z = g_zBuffer.Load(int3(ssP, 0)).r * cameraStatic.zFar;
	return reconstructCSPosition(float2(ssP), z);
}

void main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	int2 texelCoord = int2(vertexID, instanceID);

	float3 positionVS = getPosition(texelCoord);
	float3 positionWS = mul(cameraBuffer.invViewMatrix, float4(positionVS, 1)).xyz;

	for (uint i = 0; i < g_lightCount; ++i)
	{
		uint cmp = testLight(i, positionWS);

		if (cmp != 0)
		{
			g_visibleLights[i] = 1;
			break;
		}
	}
}