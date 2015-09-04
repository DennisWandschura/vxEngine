struct GSOutput
{
	float4 pos : SV_POSITION;
	uint slice : SV_RenderTargetArrayIndex;
	float distanceToLight : BLENDINDICES1;
};

float main(GSOutput input) : SV_TARGET
{
	return input.distanceToLight;
}