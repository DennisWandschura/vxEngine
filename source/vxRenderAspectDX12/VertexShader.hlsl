struct Vertex
{
	float3 position : POSITION0;
	float3 color : COLOR0;
};

struct PSIN
{
	float4 position : SV_POSITION;
	float3 color : COLOR0;
};

struct CameraData
{
	float4 cameraPosition;
	float4x4 pvMatrix;
	float4x4 cameraViewMatrix;
};

struct Transform
{
	float4 translation;
};

cbuffer CameraBuffer : register(b0)
{
	CameraData cameraBuffer;
};

//StructuredBuffer<Transform> transformBuffer : register(t0);

PSIN main(Vertex vsin )
{
	float4 wsPosition = float4(vsin.position, 1);// +transformBuffer[vsin.index].translation.xyz;

	PSIN vsout;
	vsout.position = mul(cameraBuffer.pvMatrix, wsPosition);
	vsout.color = vsin.color;
	//vsout.position = wsPosition;

	return vsout;
}