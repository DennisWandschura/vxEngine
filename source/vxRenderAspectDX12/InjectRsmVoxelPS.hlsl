#include "gpumath.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	int3 gridPosition : POSITION0;
	int axis : BLENDINDICES0;
	float3 color : COLOR0;
};

RWTexture3D<uint> g_voxelTextureDiffuse : register(u1);

void main(VSOutput input)
{
	float luminance = getLuminance(input.color);
	uint packedColor = packR8G8B8A8(float4(input.color, luminance));
	uint oldValue;
	InterlockedMax(g_voxelTextureDiffuse[(uint3)input.gridPosition], packedColor, oldValue);
}