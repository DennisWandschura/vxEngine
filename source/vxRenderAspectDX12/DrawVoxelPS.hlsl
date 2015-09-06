struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 color : NORMAL0;
};

float4 main(GSOutput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}