#include "GpuMath.h"
#include "GpuLight.h"
#include "GpuCameraBufferData.h"
#include "GpuShadowTransform.h"

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

StructuredBuffer<GpuLight> g_lights : register(t4);

Texture2DArray g_albedoeSlice : register(t0);
Texture2DArray g_normalSlice : register(t1);
Texture2D<float> g_zBuffer : register(t2);
Texture2DArray g_surfaceSlice : register(t3);

TextureCube<float> g_shadowTexture : register(t5);
TextureCube<half> g_shadowTextureIntensity : register(t6);

SamplerState g_sampler : register(s0);
SamplerState g_samplerShadow : register(s1);

float sampleShadow(in float3 L, float distance, float lightFalloff)
{
	float3 position_ls = -L;
	position_ls.x = -position_ls.x;

	float cmp = distance / lightFalloff;
	float sampledDepth = g_shadowTexture.Sample(g_samplerShadow, position_ls).r;
	return (cmp < sampledDepth) ? 1 : 0;
}

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraBuffer.projInfo.xy + cameraBuffer.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP)
{
	float3 P;

	P.z = g_zBuffer.Load(int3(ssP, 0)).r * cameraBuffer.zFar;

	P = reconstructCSPosition(float2(ssP), P.z);

	return P;
}

float getFalloff(float distance, float lightFalloff)
{
	float result = clamp(1.0 - pow(distance / lightFalloff, 4.0), 0.0, 1.0);
	return result * result / (distance * distance + 1.0);
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
	float3 albedoColor = g_albedoeSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).rgb;
	float2 packedNormal = g_normalSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).rg;
	float2 surfaceSlice = g_surfaceSlice.Sample(g_sampler, float3(input.texCoords, 0.0)).rg;

	float specularReflectance = surfaceSlice.r;
	float roughness = surfaceSlice.g;

	float3 vsNormal = decodeNormal(packedNormal.rg);
	float3 wsNormal = mul(cameraBuffer.invViewMatrix, float4(vsNormal, 0)).xyz;

	int2 ssC = int2(input.pos.xy);

	float3 positionVS = getPosition(ssC);
	float3 positionWS = mul(cameraBuffer.invViewMatrix, float4(positionVS, 1)).xyz;

	float3 v = normalize(-positionVS);

	float3 shadedColor = 0.0f;
	uint lightCount = asuint(g_lights[0].position.x);

	for (uint i = 0; i < lightCount; ++i)
	{
		//uint i = 0;
		uint index = i + 1;
		float3 lightPosition = g_lights[index].position.xyz;
		float3 lightPositionVS = mul(cameraBuffer.viewMatrix, float4(lightPosition, 1)).xyz;

		float lightFalloff = g_lights[index].falloff;
		float lightLumen = g_lights[index].lumen;

		float3 LWS = lightPosition - positionWS;
		float distance = length(LWS);

		if (distance < lightFalloff)
		{
			float3 LVS = lightPositionVS - positionVS;
			float3 l = normalize(LVS);

			//float3 l = L / distance;

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
			if (i == 0)
			{
				visibility = sampleShadow(LWS, distance, lightFalloff);
			}
	
			shadedColor += (specularColor * nDotL + diffuseColor) * lightIntensity * visibility;
		}
	}

	return float4(shadedColor, 1.0f);
}