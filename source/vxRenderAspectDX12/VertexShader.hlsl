struct Vertex
{
	float3 position : POSITION0;
	int index : BLENDINDICES0;
};

struct VSOUT
{
	float4 position : SV_POSITION;
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

StructuredBuffer<Transform> transformBuffer : register(t0);

VSOUT main(Vertex vsin )
{
	float3 wsPosition = vsin.position;// +transformBuffer[vsin.index].translation.xyz;

	VSOUT vsout;
	vsout.position = mul(float4(wsPosition, 1), cameraBuffer.pvMatrix);

	return vsout;
}