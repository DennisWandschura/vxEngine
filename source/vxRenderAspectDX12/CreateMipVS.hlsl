struct VSOutput
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

Texture2D g_srcTexture : register(t0);

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	int2 texelPosDst = int2(vertexID, instanceID);
	int2 texelPosSrc = texelPosDst * 2;

	uint srcw, srch;
	g_srcTexture.GetDimensions(srcw, srch);

	uint w = srcw / 2;
	uint h = srch / 2;

	float2 screenPos = float2(texelPosDst) / float2(w, h) * float2(2, -2) - float2(1, -1);

	float4 value0 = g_srcTexture.Load(int3(texelPosSrc, 0));
	float4 value1 = g_srcTexture.Load(int3(texelPosSrc + int2(0, 1), 0));
	float4 value2 = g_srcTexture.Load(int3(texelPosSrc + int2(1, 1), 0));
	float4 value3 = g_srcTexture.Load(int3(texelPosSrc + int2(1, 0), 0));

	float4 value = (value0 + value1 + value2 + value3) / 4;

	VSOutput output;
	output.position = float4(screenPos, 0, 1);
	output.color = value;

	return output;
}