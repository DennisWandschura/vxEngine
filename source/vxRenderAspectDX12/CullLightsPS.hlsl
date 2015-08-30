struct GSOutput
{
	float4 pos : SV_POSITION;
	uint lightIndex : BLENDINDEX0;
};

RWStructuredBuffer<uint> g_visibleLights : register(u0);

void main(GSOutput input)
{	
	g_visibleLights[input.lightIndex] = 1;
}