Texture3D<float4> g_srcTexture : register(t0);
RWTexture3D<float4> g_dstTexture : register(u0);

cbuffer RootBuffer : register(b0)
{
	uint g_axis;
	int g_srcMip;
};

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

	float4 color = 0;
	color += g_srcTexture.Load(int4(srcPos, 0));
	color += g_srcTexture.Load(int4(srcPos + int3(1, 0, 0), 0));
	color += g_srcTexture.Load(int4(srcPos + int3(1, 1, 0), 0));
	color += g_srcTexture.Load(int4(srcPos + int3(0, 1, 0), 0));

	color += g_srcTexture.Load(int4(srcPos + int3(0, 0, 1), 0));
	color += g_srcTexture.Load(int4(srcPos + int3(1, 0, 1), 0));
	color += g_srcTexture.Load(int4(srcPos + int3(1, 1, 1), 0));
	color += g_srcTexture.Load(int4(srcPos + int3(0, 1, 1), 0));

	color /= 8.0;
	color = clamp(color, 0, 1);

	g_dstTexture[dstPos] = color;
}