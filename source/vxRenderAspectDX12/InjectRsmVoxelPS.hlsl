#include "gpumath.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	int3 gridPosition : POSITION0;
	int axis : BLENDINDICES0;
	float3 color : COLOR0;
};

RWTexture3D<uint> g_voxelTextureOpacity : register(u0);
RWTexture3D<uint> g_voxelTextureDiffuse : register(u1);

uint packColor(float4 color)
{
	color = clamp(color, 0, 1);
	uint4 colorU8 = uint4(color * 255.0);
	uint packedColor = colorU8.x | (colorU8.y << 8) | (colorU8.z << 16) | (colorU8.w << 24);
	return packedColor;
}

void writeOpacity(uint3 gridPosition, int axis)
{
	uint mask = (1 << axis);

	uint oldValue;
	InterlockedOr(g_voxelTextureOpacity[(uint3)gridPosition], mask, oldValue);
}

void main(VSOutput input)
{
	writeOpacity(input.gridPosition, input.axis);

	float luminance = getLuminance(input.color);
	uint packedColor = packColor(float4(input.color, luminance));
	uint oldValue;
	InterlockedMax(g_voxelTextureDiffuse[(uint3)input.gridPosition], packedColor, oldValue);
}