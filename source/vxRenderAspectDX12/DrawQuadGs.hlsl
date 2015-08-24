struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

static const float2 positions[4] =
{
	{ -1, -1 },{ 1, -1 },{ 1, 1 },{ -1, 1 }
};

static const float2 texCoords[4] =
{
	{0, 1}, {1, 1}, {1, 0}, {0, 0}
};

[maxvertexcount(6)]
void main(
	point float4 input[1] : SV_POSITION, 
	inout TriangleStream< GSOutput > output
)
{
	GSOutput element;

	element.pos = float4(positions[0], 0, 1);
	element.texCoords = texCoords[0];
	output.Append(element);

	element.pos = float4(positions[1], 0, 1);
	element.texCoords = texCoords[1];
	output.Append(element);

	element.pos = float4(positions[2], 0, 1);
	element.texCoords = texCoords[2];
	output.Append(element);

	output.RestartStrip();

	element.pos = float4(positions[0], 0, 1);
	element.texCoords = texCoords[0];
	output.Append(element);

	element.pos = float4(positions[2], 0, 1);
	element.texCoords = texCoords[2];
	output.Append(element);

	element.pos = float4(positions[3], 0, 1);
	element.texCoords = texCoords[3];
	output.Append(element);

}