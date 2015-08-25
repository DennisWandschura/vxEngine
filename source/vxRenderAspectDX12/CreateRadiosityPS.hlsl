struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

Texture2DArray g_diffuseSlice : register(t0);
Texture2DArray g_normalVelocitySlice : register(t1);
Texture2DArray g_depthSlice : register(t2);
SamplerState g_sampler : register(s0);


struct PSOutput
{
	float4 normalSlice : SV_TARGET0;
	float4 layer0 : SV_TARGET1;
	float4 layer1 : SV_TARGET2;
	float layer0ViewZ : SV_TARGET3;
	float layer1ViewZ : SV_TARGET4;
};

float4 main(GSOutput input) : SV_TARGET
{

	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}