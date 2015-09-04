#include "GpuShadowTransform.h"

struct VSOutput
{
	float3 wsPosition : POSITION0;
	uint lightIndex : BLENDINDICES0;
	float distanceToLight : BLENDINDICES1;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float distanceToLight : BLENDINDICES1;
};

StructuredBuffer<ShadowTransform> shadowTransforms : register(t1);

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

			element.slice = slice;
			element.distanceToLight = input[i].distanceToLight;
			output.Append(element);
		}
		output.RestartStrip();
	}
}