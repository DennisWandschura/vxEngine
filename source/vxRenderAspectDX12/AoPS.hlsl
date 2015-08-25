#include "GpuMath.h"

Texture2DArray g_diffuseSlice : register(t0);
Texture2DArray g_normalVelocitySlice : register(t1);
Texture2DArray g_depthSlice : register(t2);
SamplerState g_sampler : register(s0);

float getAo(float3 wsPositionA, float3 normalA, float3 wsPositionB, float r, float beta)
{
	const float epsilon = 0.001;

	float3 v = normalize(wsPositionB - wsPositionA);

	float vDotV = dot(v, v);

	float term0 = 1.0 - vDotV / (r * r);

	float term1 = (dot(v, normalA) - beta) / sqrt(vDotV + epsilon);
	term1 = max(term1, 0.0);

	return term0 * term1;
}

float4 main() : SV_TARGET
{


	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}