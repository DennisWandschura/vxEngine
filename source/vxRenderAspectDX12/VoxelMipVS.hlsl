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

	uint color0 = toU32(g_srcTexture.Load(int4(srcPos, 0)));
	uint color1 = toU32(g_srcTexture.Load(int4(srcPos + int3(1, 0, 0), 0)));
	uint color2 = toU32(g_srcTexture.Load(int4(srcPos + int3(1, 1, 0), 0)));
	uint color3 = toU32(g_srcTexture.Load(int4(srcPos + int3(0, 1, 0), 0)));

	uint color4 = toU32(g_srcTexture.Load(int4(srcPos + int3(0, 0, 1), 0)));
	uint color5 = toU32(g_srcTexture.Load(int4(srcPos + int3(1, 0, 1), 0)));
	uint color6 = toU32(g_srcTexture.Load(int4(srcPos + int3(1, 1, 1), 0)));
	uint color7 = toU32(g_srcTexture.Load(int4(srcPos + int3(0, 1, 1), 0)));

	uint color00 = max(color0, max(color1, max(color2, color3)));
	uint color11 = max(color4, max(color5, max(color6, color7)));

	g_dstTexture[dstPos] = max(color00, color11);
}