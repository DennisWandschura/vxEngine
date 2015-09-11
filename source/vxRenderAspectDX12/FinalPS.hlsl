#include "gpumath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D<float4> g_saoTexture : register(t0);
Texture2D<half4> g_diffuseTexture : register(t1);
Texture2D<half4> g_indirectTexture : register(t2);
Texture2DArray<float4> g_albedoTexture : register(t3);

SamplerState g_sampler : register(s0);

static const float whitePoint = 3.0;
static const float whitePoint2 = whitePoint * whitePoint;

float3 tonemap(float3 color)
{
	float3 tmp = 1 + color / whitePoint2;
	return color * tmp / (1 + color);
}

float4 main(GSOutput input) : SV_TARGET
{
	int2 ssC = int2(input.pos.xy);

	float sao = g_saoTexture.Sample(g_sampler, input.texCoords).r;
	float4 shadedColor = g_diffuseTexture.Sample(g_sampler, input.texCoords).rgba;
	float3 indirectColor = g_indirectTexture.Sample(g_sampler, input.texCoords).rgb;
	float3 albedo = g_albedoTexture.Sample(g_sampler, float3(input.texCoords, 0)).rgb;

	float3 finalColor = shadedColor.rgb + indirectColor * albedo;
	finalColor = tonemap(finalColor);

	return float4(finalColor, 1.0f);
}