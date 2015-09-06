struct VSOutput
{
	float4 position : SV_POSITION;
	uint2 texelCoord : BLENDINDICES0;
};

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	float2 coords = float2(vertexID, instanceID) / float2(1920, 1080);
	coords.x = coords.x * 2.0 - 1.0;
	coords.y = 1.0 - coords.y * 2.0;

	VSOutput output;
	output.position = float4(coords, 0, 1);
	output.texelCoord = uint2(vertexID, instanceID);

	return output;
}