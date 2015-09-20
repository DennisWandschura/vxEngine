#include "gpumath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D<float4> g_voxelIndirect : register(t0);
SamplerState g_sampler : register(s0);

cbuffer RootBuffer : register(b0)
{
	uint pixelDist;
};

float4 main(GSOutput input) : SV_TARGET
{
	uint w, h;
	g_voxelIndirect.GetDimensions(w, h);

	float2 invDim = 1.0 / float2(w, h);

float4 color = 0;
float sum = 0.0;
float2 offset = invDim * pixelDist;

[unroll]
for (int y = 0; y < 2; ++y)
{
	[unroll]
	for (int x = 0; x < 2; ++x)
	{
		float4 sampleColor = g_voxelIndirect.Sample(g_sampler, input.texCoords + offset * float2(x, y));
		color += sampleColor;
	}
}
return color / 4.0;
}