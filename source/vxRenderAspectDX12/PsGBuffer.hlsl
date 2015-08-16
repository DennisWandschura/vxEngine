struct Output
{
	float4 diffuseSlice : SV_TARGET0;
	float4 normalSlice : SV_TARGET1;
	float4 tangentSlice : SV_TARGET2;
	float4 surfaceSlice : SV_TARGET3;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float3 vsPosition : POSITION1;
	float3 normal : NORMAL0;
	float3 tangent : TANGENT0;
	float2 uv : TEXCOORD0;
	uint materialId : BLENDINDICES0;
};

struct Material
{
	uint textureSlices;
};

StructuredBuffer<Material> s_materials : register(t1);
Texture2DArray g_textureSrgba : register(t2);
Texture2DArray g_textureRgba : register(t3);
SamplerState g_sampler : register(s0);

uint3 getTextureSlices(uint materialId)
{
	uint packedSlices = s_materials[materialId].textureSlices;

	uint3 result;
	// diffuse
	result.x = packedSlices & 0xff;
	// normal
	result.y = (packedSlices >> 8) & 0xff;
	// surface
	result.z = (packedSlices >> 16) & 0xff;

	return result;
}

float2 encodeNormal(float3 n)
{
	float f = sqrt(8 * n.z + 8);
	return n.xy / f + 0.5;
}

float3 decodeNormal(float2 enc)
{
	float2 fenc = enc * 4 - 2;
	float f = dot(fenc, fenc);
	float g = sqrt(1 - f / 4);
	float3 n;
	n.xy = fenc*g;
	n.z = 1 - f / 2;
	return n;
}

Output main(PSInput input)
{
	uint3 textureSlices = getTextureSlices(input.materialId);

	float4 diffuseColor = g_textureSrgba.Sample(g_sampler, float3(input.uv, float(textureSlices.x)));
	float3 normalMap = g_textureRgba.Sample(g_sampler, float3(input.uv, float(textureSlices.y))).xyz;

	float linearViewZ = length(input.vsPosition);
	float2 compressedNormal = float2(normalMap.xy);
	float2 compressedTangent = float2(0, 0);
	float2 compressedBitangent = float2(0, 0);

	Output output;
	output.diffuseSlice = diffuseColor;
	output.normalSlice = float4(compressedNormal, linearViewZ, 0);
	output.tangentSlice = float4(compressedTangent, compressedBitangent);
	output.surfaceSlice = float4(0, 0, 0, 0);

	return output;
}