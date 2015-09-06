struct GSInput
{
	float4 position : SV_POSITION;
	float4 positionPrev : POSITION1;
	float3 vsNormal : NORMAL0;
	float3 vsTangent : NORMAL1;
	float3 vsBitangent : NORMAL2;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 positionPrev : POSITION1;
	float3 vsNormal : NORMAL0;
	float3 vsTangent : NORMAL1;
	float3 vsBitangent : NORMAL2;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
	uint slice : SV_RenderTargetArrayIndex;
};

[maxvertexcount(6)]
void main(
	triangle GSInput input[3],
	inout TriangleStream< PSInput > output
	)
{
	PSInput element;

	element.slice = 0;
	for (uint i = 0; i < 3; i++)
	{
		element.position = input[i].position;
		element.positionPrev = input[i].positionPrev;
		element.vsNormal = input[i].vsNormal;
		element.vsTangent = input[i].vsTangent;
		element.vsBitangent = input[i].vsBitangent;
		element.texCoords = input[i].texCoords;
		element.material = input[i].material;

		output.Append(element);
	}
	output.RestartStrip();

	element.slice = 1;
	for (uint j = 0; j < 3; ++j)
	{
		element.position = input[j].positionPrev;
		element.positionPrev = input[j].positionPrev;
		element.vsNormal = input[j].vsNormal;
		element.vsTangent = input[j].vsTangent;
		element.vsBitangent = input[j].vsBitangent;
		element.texCoords = input[j].texCoords;
		element.material = input[j].material;

		output.Append(element);
	}
}