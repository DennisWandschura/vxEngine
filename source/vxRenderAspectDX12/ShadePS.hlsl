#include "GpuMath.h"
#include "GpuLight.h"
#include "GpuCameraBufferData.h"
#include "GpuShadowTransform.h"

struct GSOutput
{
	float4 position : SV_POSITION;
	float3 wsPosition : POSITION0;
	float3 vsPosition : POSITION1;
	float2 falloffLumen : BLENDINDICES0;
	uint lightIndex : BLENDINDICES1;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

cbuffer CameraStaticBuffer : register(b1)
{
	GpuCameraStatic cameraStatic;
};

Texture2D g_albedoeSlice : register(t1);
Texture2D<float> g_zBuffer : register(t2);
Texture2D g_normalSlice : register(t3);
Texture2D g_surfaceSlice : register(t4);
//TextureCubeArray<float2> g_shadowTextureLinear : register(t5);

SamplerState g_sampler : register(s0);
SamplerState g_samplerShadow : register(s1);

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraStatic.projInfo.xy + cameraStatic.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP)
{
	float3 P;

	P.z = g_zBuffer.Load(int3(ssP, 0)).r * cameraStatic.zFar;

	P = reconstructCSPosition(float2(ssP), P.z);

	return P;
}

float getFalloff(float distance, float lightFalloff)
{
	float result = clamp(1.0 - pow(distance / lightFalloff, 4.0), 0.0, 1.0);
	return result * result / (distance * distance + 1.0);
}

float ChebyshevUpperBound(float2 Moments, float t)
{
	const float g_MinVariance = 0.000001;

	float p = (t <= Moments.x);
	float Variance = Moments.y - (Moments.x * Moments.x);
	Variance = max(Variance, g_MinVariance);
	float d = t - Moments.x;
	float p_max = Variance / (Variance + d*d);
	return max(p, p_max);
}

float sampleShadow(in float3 L, float distance, float lightFalloff, uint lightIndex)
{
	//float cmp = distance / lightFalloff;
	//float2 sampledDepth = g_shadowTextureLinear.Sample(g_sampler, float4(-L.x, -L.y, L.z, lightIndex)).rg;

	//return ChebyshevUpperBound(sampledDepth, cmp);
	return 1.0;
}

float ggx(float alpha, float nDotH)
{
	float a2 = alpha * alpha;
	float dot2 = nDotH * nDotH;

	return a2 / (g_PI * pow(dot2 * (a2 - 1.0) + 1.0, 2.0));
}

float geometrySmithHelper(float a2, float nDotX)
{
	return 1.0 / (nDotX + sqrt(a2 + (1.0 - a2) * nDotX * nDotX));
}

float geometry_smith(float nDotL, float nDotV, float a2)
{
	return geometrySmithHelper(a2, nDotL) * geometrySmithHelper(a2, nDotV);
}

float fresnel(in float f0, float vDotH)
{
	return f0 + (1.0 - f0) * pow(2.0, (-5.55473 * vDotH - 6.98316) * vDotH);
}

float getSpecular(float roughness, float nDotH, float nDotL, float nDotV, float vDotH, in float f0)
{
	float alpha = roughness * roughness;

	float D = ggx(alpha, nDotH);
	float G = geometry_smith(nDotL, nDotV, alpha);
	float F = fresnel(f0, vDotH);

	return D * G * F;
}

float4 main(GSOutput input) : SV_TARGET
{
	int2 ssC = int2(input.position.xy);
	float2 texCoords = float2(ssC) / cameraStatic.resolution;

	float3 positionVS = getPosition(ssC);
	float3 positionWS = mul(cameraBuffer.invViewMatrix, float4(positionVS, 1)).xyz;

	float lightFalloff = input.falloffLumen.x;

	float3 LWS = input.wsPosition - positionWS;
	float distance = length(LWS);

	if (distance >= lightFalloff)
		discard;

	float3 albedoColor = g_albedoeSlice.Sample(g_sampler, texCoords).rgb;
	float2 packedNormal = g_normalSlice.Sample(g_sampler, texCoords).rg;
	float2 surfaceSlice = g_surfaceSlice.Sample(g_sampler, texCoords).rg;

	float lightLumen = input.falloffLumen.y;

	float specularReflectance = surfaceSlice.r;
	float roughness = surfaceSlice.g;

	float3 vsNormal = decodeNormal(packedNormal.rg);
	float3 wsNormal = mul(cameraBuffer.invViewMatrix, float4(vsNormal, 0)).xyz;

	float3 v = normalize(-positionVS);

	float3 LVS = input.vsPosition - positionVS;
	float3 l = normalize(LVS);

	float falloff = getFalloff(distance, lightFalloff);

	float nDotL = clamp(dot(vsNormal, l), 0, 1);
	float nDotV = dot(vsNormal, v);

	float3 h = normalize(v + l);
	float nDotH = clamp(dot(vsNormal, h), 0.0, 1.0);
	float vDotH = clamp(dot(v, h), 0.0, 1.0);

	float lightIntensity = falloff * lightLumen / (4 * g_PI);

	float3 specularColor = getSpecular(roughness, nDotH, nDotL, nDotV, vDotH, specularReflectance);

	float3 diffuseColor = albedoColor / g_PI * (1.0 - fresnel(specularReflectance, nDotL));

	float visibility = sampleShadow(LWS, distance, lightFalloff, input.lightIndex);
	float3 shadedColor = (specularColor * nDotL + diffuseColor) * lightIntensity * visibility;

	return float4(shadedColor, 1.0f);
}