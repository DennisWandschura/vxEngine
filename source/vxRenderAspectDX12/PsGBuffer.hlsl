#include "GpuMath.h"
#include "GpuCameraBufferData.h"

struct Output
{
	float4 diffuseSlice : SV_TARGET0;
	half4 normalVelocitySlice : SV_TARGET1;
};

struct PSInput
{
	float4 position : SV_POSITION;
	float4 positionPrev : POSITION1;
	//float3 vsPosition : POSITION2;
	float3 vsNormal : NORMAL0;
	float2 texCoords : TEXCOORD0;
	uint material : BLENDINDICES0;
	uint slice : SV_RenderTargetArrayIndex;
};

cbuffer CameraBuffer : register(b0)
{
	GpuCameraBufferData cameraBuffer;
};

Texture2DArray g_textureSrgba : register(t3);
Texture2DArray g_textureRgba : register(t4);
SamplerState g_sampler : register(s0);

uint3 getTextureSlices(uint packedSlices)
{
	uint3 result;
	result.x = packedSlices & 0xff;
	result.y = (packedSlices >> 8) & 0xff;
	result.z = (packedSlices >> 16) & 0xff;

	return result;
}

Output main(PSInput input)
{
	uint3 textureSlices = getTextureSlices(input.material);

	float4 diffuseColor = g_textureSrgba.Sample(g_sampler, float3(input.texCoords, float(textureSlices.x)));
	//float3 normalMap = g_textureRgba.Sample(g_sampler, float3(input.texCoords, float(textureSlices.y))).rgb;
	//float3x3 tbnMatrix;

	// rhs coord system
	//float view_z = -input.vsPosition.z / cameraBuffer.zFar;

	float2 compressedNormal = encodeNormal(input.vsNormal);

	float2 velocity = input.position.xy - input.positionPrev.xy;

	Output output;
	output.diffuseSlice = diffuseColor;
	output.normalVelocitySlice = float4(compressedNormal, velocity);

	return output;
}