struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D g_saoTexture : register(t0);
Texture2D g_diffuseTexture : register(t1);
SamplerState g_sampler : register(s0);

float4 main(GSOutput input) : SV_TARGET
{
	int2 ssC = int2(input.pos.xy);

	float visibility = g_saoTexture.Sample(g_sampler, input.texCoords).r;
	float3 shadedColor = g_diffuseTexture.Sample(g_sampler, input.texCoords).rgb;

	return float4(shadedColor * visibility, 1.0f);
}