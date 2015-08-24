#include "GpuCameraBufferData.h"

struct GSInput
{
	float3 wsPosition : POSITION0;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
	uint slice : SV_RenderTargetArrayIndex;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

[maxvertexcount(3)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< PSInput > output
)
{
	PSInput element;
	element.slice = 0;
	for (uint i = 0; i < 3; i++)
	{
		element.position = mul(cameraBuffer.pvMatrix, float4(input[i].wsPosition, 1));
		element.vsNormal = input[i].vsNormal;
		element.texCoords = input[i].texCoords;
		element.material = input[i].material;

		output.Append(element);
	}
	//output.RestartStrip();
}