struct GSOutput
{
	float4 pos : SV_POSITION;
	uint lightIndex : BLENDINDICES0;
};

RWStructuredBuffer<uint> g_visibleLights : register(u1);

void main(GSOutput input)
{	
	g_visibleLights[input.lightIndex] = 1;
}