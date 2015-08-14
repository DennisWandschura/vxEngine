struct Vertex
{
	float3 position : POSITION0;
	float2 texCoords : TEXCOORD0;
	uint index : BLENDINDICES0;
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

StructuredBuffer<Transform> s_transforms : register(t0);

PSIN main(Vertex vsin )
{
	float3 translation = s_transforms[vsin.index].translation.xyz;

	float4 wsPosition = float4(vsin.position + translation, 1); //

	float ccc = float(vsin.index) / 128.0;

	PSIN vsout;
	vsout.position = mul(cameraBuffer.pvMatrix, wsPosition);
	vsout.color = float3(vsin.texCoords, ccc);
	//vsout.position = wsPosition;

	return vsout;
}