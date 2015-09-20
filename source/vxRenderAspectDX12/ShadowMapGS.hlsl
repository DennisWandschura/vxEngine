#include "GpuShadowTransform.h"

struct VSOutput
{
	float3 wsPosition : POSITION0;
	float3 lightPositionWS : POSITION1;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint lightIndex : BLENDINDICES0;
	uint material : BLENDINDICES1;
	float lightFalloff : BLENDINDICES3;
	float lightLumen : BLENDINDICES4;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	float3 lightPositionWS : POSITION1;
	float3 normal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint slice : SV_RenderTargetArrayIndex;
	uint material : BLENDINDICES0;
	float lightFalloff : BLENDINDICES2;
	float lightLumen : BLENDINDICES3;
};

StructuredBuffer<GpuShadowTransform> shadowTransforms : register(t3);

[maxvertexcount(3 * 6)]
void main(
	triangle VSOutput input[3],
	inout TriangleStream< GSOutput > output
)
{
	uint lightIndex = input[0].lightIndex;

	for (uint slice = 0; slice < 6; ++slice)
	{
		float4x4 pvMatrix = shadowTransforms[lightIndex].pvMatrix[slice];

		for (uint i = 0; i < 3; i++)
		{
			GSOutput element;
			element.pos = mul(pvMatrix, float4(input[i].wsPosition, 1));
			element.wsPosition = input[i].wsPosition;
			element.lightPositionWS = input[i].lightPositionWS;
			element.normal = input[i].normal;
			element.texCoords = input[i].texCoords;
			element.slice = slice;
			element.material = input[i].material;
			element.lightFalloff = input[i].lightFalloff;
			element.lightLumen = input[i].lightLumen;
			output.Append(element);
		}
		output.RestartStrip();
	}
}