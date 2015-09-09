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
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

cbuffer CameraStaticBuffer : register(b1)
{
	GpuCameraStatic cameraStatic;
};

Texture2DArray g_albedoeSlice : register(t1);
Texture2D<float> g_zBuffer : register(t2);
Texture2DArray g_normalSlice : register(t3);
Texture2DArray g_surfaceSlice : register(t4);

//TextureCube<float> g_shadowTexture : register(t5);
//TextureCube<half> g_shadowTextureIntensity : register(t6);

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

/*float sampleShadow(in float3 L, float distance, float lightFalloff)
{
	float3 position_ls = -L;
	position_ls.x = -position_ls.x;

	float cmp = distance / lightFalloff;
	float sampledDepth = g_shadowTexture.Sample(g_samplerShadow, position_ls).r;
	return (cmp < sampledDepth) ? 1 : 0;
}
*/

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
	float3 lightPosition = input.wsPosition;

	float3 LWS = lightPosition - positionWS;
	float distance = length(LWS);
	if (distance >= lightFalloff)
		discard;

	float3 albedoColor = g_albedoeSlice.Sample(g_sampler, float3(texCoords, 0.0)).rgb;
	float2 packedNormal = g_normalSlice.Sample(g_sampler, float3(texCoords, 0.0)).rg;
	float2 surfaceSlice = g_surfaceSlice.Sample(g_sampler, float3(texCoords, 0.0)).rg;

	float lightLumen = input.falloffLumen.y;

	float specularReflectance = surfaceSlice.r;
	float roughness = surfaceSlice.g;

	float3 vsNormal = decodeNormal(packedNormal.rg);
	float3 wsNormal = mul(cameraBuffer.invViewMatrix, float4(vsNormal, 0)).xyz;

	float3 v = normalize(-positionVS);

	float3 shadedColor = 0.0f;

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

	float3 diffuseColor = albedoColor / g_PI *(1.0 - fresnel(specularReflectance, nDotL));

	float visibility = 1;
			//if (i == 0)
			//{
			//	visibility = sampleShadow(LWS, distance, lightFalloff);
			//}

	shadedColor = (specularColor * nDotL + diffuseColor) * lightIntensity * visibility;

	return float4(shadedColor, 1.0f);
}