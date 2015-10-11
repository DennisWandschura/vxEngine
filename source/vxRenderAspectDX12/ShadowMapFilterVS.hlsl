struct VSOutput
{
	uint lightIndex : BLENDINDICES0;
	uint cubeIndex : BLENDINDICES1;
};

VSOutput main(uint index : SV_VertexID, uint cubeIndex : SV_InstanceID )
{
	VSOutput output;
	output.lightIndex = index;
	output.cubeIndex = cubeIndex;

	return output;
}