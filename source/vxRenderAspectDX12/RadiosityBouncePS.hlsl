#include "gpumath.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D indirectBuffer : register(t0);
Texture2D directBuffer : register(t1);
Texture2DArray albedoBuffer : register(t2);

float colorBoost(float3 color, float unsaturatedBoost, float saturatedBoost) 
{
	// Avoid computing the HSV transform in the common case
	if (unsaturatedBoost == saturatedBoost) 
	{
		return unsaturatedBoost;
	}

	float ma = max(color.x, max(color.y, color.z));
	float mi = min(color.x, min(color.y, color.z));
	float saturation = (ma == 0.0f) ? 0.0f : ((ma - mi) / ma);

	return lerp(unsaturatedBoost, saturatedBoost, saturation);
}

float4 main(GSOutput input) : SV_TARGET
{
	int2 C = int2(input.pos.xy);

	float4 indirect = indirectBuffer.Load(int3(C, 0)).rgba;
	float3 direct = directBuffer.Load(int3(C, 0)).rgb;
	float3 albedo = albedoBuffer.Load(int4(C, 0, 0)).rgb;

	float3 finalColor = direct + indirect.rgb / g_PI * albedo;// *colorBoost(indirect, unsaturatedLightBoost, saturatedLightBoost);

	return float4(finalColor, indirect.a);
}