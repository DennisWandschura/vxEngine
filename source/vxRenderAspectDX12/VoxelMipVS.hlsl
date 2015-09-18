Texture3D<float4> g_srcTexture : register(t0);
RWTexture3D<float4> g_dstTexture : register(u0);

cbuffer RootBuffer : register(b1)
{
	int axis;
};

static const int3x3 g_swizzleMatrixes[3] = 
{
	int3x3(0, 0, 1, 0, 1, 0, 1, 0, 0),
	int3x3(1, 0, 0,
		0, 0, 1,
		0, 1, 0), 
	int3x3(1, 0, 0,
		0, 1, 0,
		0, 0, 1)
};

static const int3 g_offsets[]=
{
	int3(0, 1, 0),
	int3(1, 1, 0),
	int3(1, 0, 0)
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

	int3 offsets[3];
	offsets[0] = mul(g_offsets[0], g_swizzleMatrixes[axis]);
	offsets[1] = mul(g_offsets[1], g_swizzleMatrixes[axis]);
	offsets[2] = mul(g_offsets[2], g_swizzleMatrixes[axis]);

	int3 srcPos = dstPos * 2;

	float4 color0 = g_srcTexture.Load(int4(srcPos, 0));
	float4 color1 = g_srcTexture.Load(int4(srcPos + offsets[0], 0));
	float4 color2 = g_srcTexture.Load(int4(srcPos + offsets[1], 0));
	float4 color3 = g_srcTexture.Load(int4(srcPos + offsets[2], 0));

	float4 color = (color0 + color1 + color2 + color3) / 4.0;

	g_dstTexture[dstPos] = packColor(color);
}