#include "gpumath.h"

struct GSOutput
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORDS0;
};

struct RSMTexel
{
	float3 color;
	float3 normal;
	float depth;
};

Texture2D g_srcRSMColor : register(t0);
Texture2D<half2> g_srcRSMNormal : register(t1);
Texture2D<float> g_srcRSMDepth : register(t2);
SamplerState g_sampler : register(s0);

RSMTexel fetchRSM(half2 vTexCoords)
{
	RSMTexel result;
	result.color = g_srcRSMColor.Sample(g_sampler, vTexCoords).rgb;
	result.normal = decodeNormal(g_srcRSMNormal.Sample(g_sampler, vTexCoords));
	result.depth = g_srcRSMDepth.Sample(g_sampler, vTexCoords);

	return result;
}
/*
half3 GetGridCell(const in half2 texCoord, const in float fDepth) 
{   
	// calc grid cell pos   
	float4 texelPos = float4(texCoord * half2(2.h, -2.h) - half2(1.h, -1.h), fDepth, 1.f);   
	float4 homogWorldPos = mul(g_invRSMMatrix, texelPos);   
	return GetGridPos(homogWorldPos); 
} */

half GetTexelLum(const in RSMTexel texel, const in float3 lightDir) 
{
	return getLuminance(texel.color) * max(0.h, dot(texel.normal, lightDir));
}

float4 main(GSOutput input) : SV_TARGET
{
	/*uint w, h;
	g_srcTexture.GetDimensions(w, h);

	float2 invDim = 1.0 / float2(w, h);

	// choose the brightest texel
	half3 vChosenGridCell = 0;
	{
		half fMaxLum = 0;
		for (int i = 0;i < 2;i++)
		{
			for (int j = 0; j < 2;j++)
			{
				half2 vTexCoords = input.texCoords + half2(i, j) * invDim;
				RSMTexel texel = fetchRSM(vTexCoords);
				half fCurTexLum = GetTexelLum(texel);
				if (fCurTexLum > fMaxLum)
				{
					vChosenGridCell = GetGridCell(vTexCoords, texel.fDepth);
					fMaxLum = fCurTexLum;
				}
			}
		}
	}*/

	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}