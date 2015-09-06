RWTexture3D<float> g_voxelTextureSrc : register(u0);
RWTexture3D<float> g_voxelTextureDst : register(u1);

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

	float value0 = g_voxelTextureSrc[voxelPosSrc];
	float value1 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 0, 0)];
	float value2 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 1, 0)];
	float value3 = g_voxelTextureSrc[voxelPosSrc + uint3(0, 1, 0)];

	value0 = max(value0, value1);
	value2 = max(value2, value3);
	value0 = max(value0, value2);

	float value4 = g_voxelTextureSrc[voxelPosSrc + uint3(0, 0, 1)];
	float value5 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 0, 1)];
	float value6 = g_voxelTextureSrc[voxelPosSrc + uint3(1, 1, 1)];
	float value7 = g_voxelTextureSrc[voxelPosSrc + uint3(0, 1, 1)];

	value4 = max(value4, value5);
	value6 = max(value6, value7);
	value4 = max(value4, value6);

	float maxValue = max(value0, value4);

	g_voxelTextureDst[voxelPosDst] = maxValue;
}