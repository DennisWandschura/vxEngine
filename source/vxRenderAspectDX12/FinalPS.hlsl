struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2D g_saoTexture : register(t0);
Texture2D g_diffuseTexture : register(t1);
//Texture2D g_voxelDebugTexture : register(t2);
SamplerState g_sampler : register(s0);

static const float whitePoint = 3.0;
static const float whitePoint2 = whitePoint * whitePoint;

float3 tonemap(float3 color)
{
	float3 tmp = 1 + color / whitePoint2;
	return color * tmp / (1 + color);
}

float4 main(GSOutput input) : SV_TARGET
{
	int2 ssC = int2(input.pos.xy);

	float sao = g_saoTexture.Sample(g_sampler, input.texCoords).r;
	float3 shadedColor = g_diffuseTexture.Sample(g_sampler, input.texCoords).rgb;
	//float4 voxelColor = g_voxelDebugTexture.Sample(g_sampler, input.texCoords);

	//shadedColor = tonemap(shadedColor * sao);


	return float4(shadedColor, 1.0f);
}