#include "gpumath.h"

Texture3D<uint4> g_srcTexture : register(t0);
RWTexture3D<uint> g_dstTexture : register(u0);

cbuffer RootBuffer : register(b0)
{
	uint g_axis;
	int g_srcMip;
};

uint toU32(in uint4 color)
{
	color = clamp(color, 0, 255);
	return (color.x | (color.y << 8) | (color.z << 16) | (color.w << 24));
}

float4 toR8G8B8A8(in uint4 color)
{
	return float4(color) / 255.0;
}

void main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint w, h, d;
	g_dstTexture.GetDimensions(w, h, d);

	uint x = vertexID;
	uint yz = instanceID;
	uint y = yz % w;
	uint z = yz / w;

	uint wOffset = g_axis * w;
	int3 dstPos = int3(x, y, z + wOffset);
	int3 srcPos = dstPos * 2;

	float4 color0 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos, 0)));
	float4 color1 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(1, 0, 0), 0)));
	float4 color2 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(1, 1, 0), 0)));
	float4 color3 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(0, 1, 0), 0)));

	float4 color4 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(0, 0, 1), 0)));
	float4 color5 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(1, 0, 1), 0)));
	float4 color6 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(1, 1, 1), 0)));
	float4 color7 = toR8G8B8A8(g_srcTexture.Load(int4(srcPos + int3(0, 1, 1), 0)));

	//uint color00 = max(color0, max(color1, max(color2, color3)));
	//uint color11 = max(color4, max(color5, max(color6, color7)));

	float4 color = color0 + color1 + color2 + color3 + color4 + color5 + color6 + color7;
	color /= 4.0;

	color = clamp(color, 0, 1);

	g_dstTexture[dstPos] = packR8G8B8A8(color);//max(color00, color11);
}