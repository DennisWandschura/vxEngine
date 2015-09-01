struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D g_saoTexture : register(t0);
Texture2D g_diffuseTexture : register(t1);
SamplerState g_sampler : register(s0);

static const float whitePoint = 2.0;
static const float whitePoint2 = whitePoint * whitePoint;

float3 tonemap(float3 color)
{
	float3 tmp = 1 + color / whitePoint2;
	return color * tmp / (1 + color);
}

float4 main(GSOutput input) : SV_TARGET
{
	int2 ssC = int2(input.pos.xy);

	float visibility = g_saoTexture.Sample(g_sampler, input.texCoords).r;
	float3 shadedColor = g_diffuseTexture.Sample(g_sampler, input.texCoords).rgb;

	shadedColor = tonemap(shadedColor * visibility);

	float3 invGamma = 1.0 / 2.2;
	shadedColor = pow(shadedColor, invGamma);

	return float4(shadedColor, 1.0f);
}