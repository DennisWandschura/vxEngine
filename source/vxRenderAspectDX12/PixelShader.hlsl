struct PSIN
{
	float4 position : SV_POSITION;
	float3 wsPosition : POSITION1;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
};

Texture2DArray g_textureSrgb : register(t2);
Texture2DArray g_textureRgb : register(t3);
SamplerState g_sampler : register(s0);

static const float g_PI = 3.141592654;
static const float3 lightPosition = float3(0.0, 2, 0);
static const float lightRadius = 5.0f;
static const float lightLumen = 50.0f;

float getFalloff(float d, float lightFalloff)
{
	float result = clamp(1.0 - pow(d / lightFalloff, 4.0), 0.0, 1.0);
	//float result = saturate(1.0 - pow(d / lightFalloff, 4.0));
	return result * result / (d * d + 1.0);
}

float lightning(float3 wsPosition)
{
	float3 L = lightPosition - wsPosition;
	float distance = length(L);
	//float3 l = L / distance;

	float lightFalloff = getFalloff(distance, lightRadius);
	float lightIntensity = lightFalloff * lightLumen / (4 * g_PI);

	return lightIntensity;
}

float4 main(PSIN input) : SV_TARGET
{
	uint diffuseSlice = input.material & 0xff;
	uint normalSlice = (input.material >> 8) & 0xff;
	uint surfaceSlice = (input.material >> 16) & 0xff;

	float4 diffuseColor = g_textureSrgb.Sample(g_sampler, float3(input.texCoords, float(diffuseSlice)));
	float4 normals = g_textureRgb.Sample(g_sampler, float3(input.texCoords, float(normalSlice)));

	const float invGamma = 1.0 / 2.2;
	diffuseColor = pow(diffuseColor, float4(invGamma, invGamma, invGamma, invGamma));

//	float lightIntensity = lightning(input.wsPosition);
	float lightIntensity = 1.0;

	//return float4(diffuseColor.xyz, 1.0f);
	return float4(diffuseColor.rgb, 1.0f);
}