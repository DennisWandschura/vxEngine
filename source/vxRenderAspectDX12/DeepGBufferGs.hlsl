#include "GpuCameraBufferData.h"

struct GSInput
{
	float3 wsPosition : POSITION0;
	//float4 positionPrev : POSITION1;
	//float3 vsPosition : POSITION2;
	float3 vsNormal : NORMAL0;
	//float3 vsTangent : TANGENT0;
	//float3 vsBitangent : BITANGENT0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	//float4 positionPrev : POSITION1;
	//float3 vsPosition : POSITION2;
	float3 vsNormal : NORMAL0;
	//float3 vsTangent : TANGENT0;
	//float3 vsBitangent : BITANGENT0;
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
		//element.positionPrev = input[i].positionPrev;
		//element.vsPosition = input[i].vsPosition;
		element.vsNormal = input[i].vsNormal;
		//element.vsTangent = input[i].vsTangent;
		//element.vsBitangent = input[i].vsBitangent;
		element.texCoords = input[i].texCoords;
		element.material = input[i].material;

		output.Append(element);
	}
	//output.RestartStrip();

	/*for (uint j = 0; j < 3; j++)
	{
		PSInput element;
		element.position = input[j].position;
		element.positionPrev = input[j].positionPrev;
		element.vsPosition = input[j].vsPosition;
		element.vsNormal = input[j].vsNormal;
		element.vsTangent = input[j].vsTangent;
		element.vsBitangent = input[j].vsBitangent;
		element.texCoords = input[j].texCoords;
		element.material = input[j].material;
		element.slice = 1;

		output.Append(element);
	}*/
}