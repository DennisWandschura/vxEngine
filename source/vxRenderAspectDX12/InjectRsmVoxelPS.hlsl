#include "gpumath.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	int3 gridPosition : POSITION0;
	int axis : BLENDINDICES0;
	float3 color : COLOR0;
	float4 normal : NORMAL0;
};

RWTexture3D<uint> g_voxelTextureDiffuse : register(u1);
RWTexture3D<uint> g_voxelTextureNormals : register(u2);

void main(VSOutput input)
{
	float luminance = getLuminance(input.color);
	uint packedColor = packR8G8B8A8(float4(input.color, luminance));
	uint oldValue;
	InterlockedMax(g_voxelTextureDiffuse[(uint3)input.gridPosition], packedColor, oldValue);

	uint packedNormal = packR8G8B8A8(input.normal);
	InterlockedMax(g_voxelTextureNormals[(uint3)input.gridPosition], packedNormal, oldValue);
}