Texture3D<uint> g_srcTexture : register(t0);
RWTexture3D<uint> g_dstTexture : register(u0);

cbuffer RootBuffer : register(b0)
{
	int axis;
};

static const int3 g_offsets[] =
{
	int3(0, 1, 0),
	int3(1, 1, 0),
	int3(1, 0, 0),

	int3(0, 0, 1),
	int3(0, 1, 1),
	int3(1, 1, 1),
	int3(1, 0, 1)
};

float4 unpackColor(uint packedColor)
{
	uint4 tmp;
	tmp.x = packedColor & 0xff;
	tmp.y = (packedColor >> 8) & 0xff;
	tmp.z = (packedColor >> 16) & 0xff;
	tmp.w = (packedColor >> 24) & 0xff;

	return float4(tmp) / 255.0;
}

uint packColor(float4 color)
{
	color = clamp(color, 0, 1);
	uint4 colorU8 = uint4(color * 255.0);
	uint packedColor = colorU8.x | (colorU8.y << 8) | (colorU8.z << 16) | (colorU8.w << 24);
	return packedColor;
}

void main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint w, h, d;
	g_dstTexture.GetDimensions(w, h, d);

	uint x = vertexID;
	uint yz = instanceID;
	uint y = yz % w;
	uint z = yz / w;

	int3 dstPos = int3(x, y, z);
	int3 srcPos = dstPos * 2;

	uint value0 = g_srcTexture.Load(int4(srcPos, 0));
	uint value1 = g_srcTexture.Load(int4(srcPos + g_offsets[0], 0));
	uint value2 = g_srcTexture.Load(int4(srcPos + g_offsets[1], 0));
	uint value3 = g_srcTexture.Load(int4(srcPos + g_offsets[2], 0));
	uint value4 = g_srcTexture.Load(int4(srcPos + g_offsets[3], 0));
	uint value5 = g_srcTexture.Load(int4(srcPos + g_offsets[4], 0));
	uint value6 = g_srcTexture.Load(int4(srcPos + g_offsets[5], 0));
	uint value7 = g_srcTexture.Load(int4(srcPos + g_offsets[6], 0));

	uint value = value0 | value1 | value2 | value3 | value4 | value5 | value6 | value7;
	value = 0xffff;

	g_dstTexture[dstPos] = value;
}