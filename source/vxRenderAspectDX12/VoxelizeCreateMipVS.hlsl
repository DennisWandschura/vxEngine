Texture3D<uint> g_voxelTextureSrc : register(t0);
RWTexture3D<uint> g_voxelTextureDst : register(u0);

float4 unpackColor(uint packedColor)
{
	uint4 color = uint4(packedColor, packedColor >> 8, packedColor >> 16, packedColor >> 24) & 0xff;

	float4 unpackedColor = float4(color) / 255.0;
	return unpackedColor;
}

uint packColor(float4 color)
{
	uint4 tmp = uint4(color * 255.0);
	tmp = clamp(tmp, 0, 255);

	return tmp.x | (tmp.y << 8) | (tmp.z << 16) | (tmp.w << 24);
}

void main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint x = vertexID;
	uint yz = instanceID;

	uint width, height, depth;
	g_voxelTextureDst.GetDimensions(width, height, depth);

	uint3 voxelPosDst;
	voxelPosDst.x = x;
	voxelPosDst.y = yz % width;
	voxelPosDst.z = yz / width;

	uint3 voxelPosSrc = voxelPosDst * 2;

	uint value0 = g_voxelTextureSrc[voxelPosSrc];
	uint value1 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 0, 0)];
	uint value2 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 1, 0)];
	uint value3 = g_voxelTextureSrc[voxelPosSrc + uint3(0, 1, 0)];

	uint value4 = g_voxelTextureSrc[voxelPosSrc + uint3(0, 0, 1)];
	uint value5 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 0, 1)];
	uint value6 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 1, 1)];
	uint value7 = g_voxelTextureSrc[voxelPosSrc + uint3(0, 1, 1)];

	float4 color = unpackColor(value0) + unpackColor(value1) + unpackColor(value2) + unpackColor(value3);
	color += unpackColor(value4) + unpackColor(value5) + unpackColor(value6) + unpackColor(value7);

	g_voxelTextureDst[voxelPosDst] = packColor(color / 8.0);
}