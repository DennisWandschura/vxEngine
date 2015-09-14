struct VSOutput
{
	uint lightIndex : BLENDINDICES0;
};

VSOutput main(uint index : SV_VertexID )
{
	VSOutput output;
	output.lightIndex = index;

	return output;
}