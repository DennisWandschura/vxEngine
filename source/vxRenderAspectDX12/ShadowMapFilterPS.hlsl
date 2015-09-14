#include "gpumath.h"
#include "GpuShadowTransform.h"
#include "gpulpv.h"

struct GSOutput
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORDS0;
	float3 dirToLight : NORMAL0;
	uint lightIndex : BLENDINDICES0;
	uint index : BLENDINDICES1;
	uint slice : SV_RenderTargetArrayIndex;
};

struct PSOutput
{
	float4 color : SV_TARGET0;
	half2 normal: SV_TARGET1;
	float depth : SV_TARGET2;
};

struct RSMTexel
{
	float3 color;
	float3 normal;
	float depth;
};

cbuffer lpvConstantBuffer : register(b0)
{
	GpuLpv g_lpv;
};

Texture2DArray g_srcRSMColor : register(t0);
Texture2DArray<half2> g_srcRSMNormal : register(t1);
Texture2DArray<float> g_srcRSMDepth : register(t2);
StructuredBuffer<GpuShadowTransformReverse> g_transforms : register(t3);

SamplerState g_sampler : register(s0);

RSMTexel fetchRSM(half2 vTexCoords, uint cubeSlice)
{
	RSMTexel result;
	result.color = g_srcRSMColor.Sample(g_sampler, float3(vTexCoords, cubeSlice)).rgb;
	result.normal = decodeNormal(g_srcRSMNormal.Sample(g_sampler, float3(vTexCoords, cubeSlice)));
	result.depth = g_srcRSMDepth.Sample(g_sampler, float3(vTexCoords, cubeSlice));

	return result;
}

half3 GetGridPos(in float3 wsPosition)
{
	float3 offset = wsPosition * g_lpv.invGridCellSize;
	return half3(int3(offset) + g_lpv.halfDim);
}

half3 GetGridCell(const in half2 texCoord, const in float fDepth, in float4x4 invPvMatrix)
{
	// calc grid cell pos
	float4 texelPos = float4(texCoord * half2(2.h, -2.h) - half2(1.h, -1.h), fDepth, 1.f);
	float4 homogWorldPos = mul(invPvMatrix, texelPos);
	return GetGridPos(homogWorldPos.xyz);
}

half GetTexelLum(const in RSMTexel texel, const in float3 lightDir)
{
	return getLuminance(texel.color) * max(0.h, dot(texel.normal, lightDir));
}

RSMTexel getRsmTexel(in float3 dirToLight, in float2 texCoords, uint lightIndex, uint index)
{
	uint w, h, d;
	g_srcRSMColor.GetDimensions(w, h, d);

	float2 invDim = 1.0 / float2(w, h);
	uint cubeSlice = lightIndex * 6 + index;

	float4x4 invPvMatrix = g_transforms[lightIndex].invPvMatrix[index];

	// choose the brightest texel
	half3 vChosenGridCell = 0;
	{
		half fMaxLum = 0;
		for (int i = 0; i < 2; i++)
		{
			for (int j = 0; j < 2; j++)
			{
				half2 vTexCoords = texCoords + half2(i, j) * invDim;
				RSMTexel texel = fetchRSM(vTexCoords, cubeSlice);
				
				half fCurTexLum = GetTexelLum(texel, dirToLight);
				if (fCurTexLum > fMaxLum)
				{
					vChosenGridCell = GetGridCell(texCoords, texel.depth, invPvMatrix);
					fMaxLum = fCurTexLum;
				}
			}
		}
	}

	RSMTexel cRes = (RSMTexel)0;
	half nSamples = 0;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			half2 vTexCoords = texCoords + half2(i, j) * invDim;
			RSMTexel texel = fetchRSM(vTexCoords, cubeSlice);
			half3 vTexelGridCell = GetGridCell(vTexCoords, texel.depth, invPvMatrix);
			half3 dGrid = vTexelGridCell - vChosenGridCell;
			if (dot(dGrid, dGrid) < 3)
			{
				cRes.depth += texel.depth;
				cRes.color += texel.color;
				cRes.normal += texel.normal;
				nSamples++;
			}
		}
	}

	// normalize   
	if (nSamples > 0)
	{
		cRes.depth /= nSamples;
		cRes.color /= 4;
		cRes.normal /= nSamples;
	}

	return cRes;
}

PSOutput main(GSOutput input)
{
	uint w, h, d;
	g_srcRSMColor.GetDimensions(w, h, d);

	RSMTexel texel = getRsmTexel(input.dirToLight, input.texCoords, input.lightIndex, input.index);

	PSOutput output;
	output.color = float4(texel.color, 1);
	output.normal = encodeNormal(texel.normal);
	output.depth = texel.depth;

	return output;
}