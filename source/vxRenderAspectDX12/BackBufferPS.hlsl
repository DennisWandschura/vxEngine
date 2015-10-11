struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2DArray<float4> g_layerGameTexture : register(t0);
Texture2DArray<float4> g_layerGameOverlay: register(t1);

SamplerState g_sampler : register(s0);

float4 main(GSOutput input) : SV_TARGET
{
	float4 color0 = g_layerGameTexture.Sample(g_sampler, float3(input.texCoords, 0));
	float4 color1 = g_layerGameOverlay.Sample(g_sampler, float3(input.texCoords, 0));

	float4 finalColor = color0 * (1 - color1.a) + color1;

	return float4(finalColor.rgb, 1);
}