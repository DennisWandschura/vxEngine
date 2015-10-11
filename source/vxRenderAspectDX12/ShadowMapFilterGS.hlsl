struct VSOutput
{
	uint lightIndex : BLENDINDICES0;
	uint cubeIndex : BLENDINDICES1;
};

struct GSOutput
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORDS0;
	float3 dirToLight : NORMAL0;
	uint lightIndex : BLENDINDICES0;
	uint index : BLENDINDICES1;
	uint slice : SV_RenderTargetArrayIndex;
};

static const float2 positions[4] =
{
	{ -1, -1 },{ 1, -1 },{ 1, 1 },{ -1, 1 }
};

static const float2 texCoords[4] =
{
	{ 0, 1 },{ 1, 1 },{ 1, 0 },{ 0, 0 }
};

static const float3 lightDirections[6] =
{
	float3(1, 0, 0),
	float3(-1, 0, 0),
	float3(0, 1, 0),
	float3(0, -1, 0),
	float3(0, 0, -1),
	float3(0, 0, 1)
};

[maxvertexcount(6)]
void main(
	point VSOutput input[1],
	inout TriangleStream< GSOutput > output
	)
{
	uint i = input[0].cubeIndex;
	GSOutput element;

	element.lightIndex = input[0].lightIndex;
	element.slice = input[0].lightIndex * 6 + i;
	element.index = i;
	element.dirToLight = -lightDirections[i];

	element.position = float4(positions[0], 0, 1);
	element.texCoords = texCoords[0];
	output.Append(element);

	element.position = float4(positions[1], 0, 1);
	element.texCoords = texCoords[1];
	output.Append(element);

	element.position = float4(positions[2], 0, 1);
	element.texCoords = texCoords[2];
	output.Append(element);

	output.RestartStrip();

	element.position = float4(positions[0], 0, 1);
	element.texCoords = texCoords[0];
	output.Append(element);

	element.position = float4(positions[2], 0, 1);
	element.texCoords = texCoords[2];
	output.Append(element);

	element.position = float4(positions[3], 0, 1);
	element.texCoords = texCoords[3];
	output.Append(element);
	output.RestartStrip();
}