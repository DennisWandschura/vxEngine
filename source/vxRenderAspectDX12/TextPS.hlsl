struct GSOutput
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
	float3 color : COLOR0;
};

static const float smoothness = 64.0;
static const float gamma = 2.2;
static const float invGamma = 1.0 / gamma;

Texture2D<float4> g_textTexture : register(t0);

SamplerState g_textSampler : register(s0);

float4 main(GSOutput input) : SV_TARGET
{
	float4 color = g_textTexture.Sample(g_textSampler, input.texCoords);

	float sdf = color.a;

	float w = clamp(smoothness * (abs(ddx(input.texCoords.x)) + abs(ddy(input.texCoords.y))), 0.0, 0.5);
	float a = smoothstep(0.5 - w, 0.5 + w, sdf);
	a = pow(a, invGamma);

	color.a = a;
	color.rgb = input.color.rgb;

	return color;
}