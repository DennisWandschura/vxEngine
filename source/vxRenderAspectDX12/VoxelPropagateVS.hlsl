#include "gpumath.h"

Texture3D<float4> g_srcTexture : register(t0);
//Texture3D<uint> g_opacity;

RWTexture3D<float4> g_dstTexture : register(u0);

cbuffer RootBuffer : register(b0)
{
	uint g_axis;
};

static const int3 g_offsets[6] =
{
	int3(1, 0, 0),
	int3(-1, 0, 0),
	int3(0, 1, 0),
	int3(0, -1, 0),
	int3(0, 0, 1),
	int3(0, 0, -1)
};

void main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint w, h, d;
	g_dstTexture.GetDimensions(w, h, d);

	uint x = vertexID;
	uint yz = instanceID;
	uint y = yz % w;
	uint z = yz / w;

	int wOffset = g_axis * w;

	int3 currentVoxelPosition = int3(x, y, z);
	currentVoxelPosition.z += wOffset;

	int3 offset = g_offsets[g_axis];

	float4 currentColor = g_srcTexture.Load(int4(currentVoxelPosition, 0));
	float4 color = currentColor;

	float3 direction = g_offsets[g_axis];

	//float weight = 1;
	for (int i = 0; i < 6; ++i)
	{
		int3 samplePosition = currentVoxelPosition + g_offsets[i];
		//float3 l = normalize(float3(currentVoxelPosition) - float3(samplePosition));
		//float nDotL = max(dot(direction, l), 0);

		float4 sampleColor = g_srcTexture.Load(int4(samplePosition, 0));// *nDotL;

		color += sampleColor;// *0.5;
		//weight += nDotL;
	}

	color /= 7.0;
	color = clamp(color, 0, 1);

	g_dstTexture[currentVoxelPosition] = color;
}