struct VSOutput
{
	int3 xyz : POSITION0;
	uint axis : AXIS0;
	float4 color : COLOR0;
};

Texture3D<float4> g_srcTexture : register(t0);

cbuffer RootBuffer : register(b0)
{
	uint g_axis;
};

VSOutput main(uint xy : SV_VertexID, uint z : SV_InstanceID)
{
	uint w, h, d;
	g_srcTexture.GetDimensions(w, h, d);

	uint x = xy % w;
	uint y = xy / w;

	int wOffset = g_axis * w;

	int3 currentVoxelPosition = int3(x, y, z);
	currentVoxelPosition.z += wOffset;

	float4 currentColor = g_srcTexture.Load(int4(currentVoxelPosition, 0));

	VSOutput output;
	output.xyz = int3(x, y, z);
	output.axis = g_axis;
	output.color = currentColor;

	return output;
}