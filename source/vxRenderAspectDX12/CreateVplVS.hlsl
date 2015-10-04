#include "gpumath.h"
#include "gpush.h"

Texture3D<float4> g_colorTexture : register(t0);
Texture3D<float4> g_normalTexture : register(t1);

RWTexture3D<half4> g_lpvRed : register(u0);
RWTexture3D<half4> g_lpvGreen : register(u1);
RWTexture3D<half4> g_lpvBlue : register(u2);

float4 clampedCosineSHCoeffs(in float3 dir)
{
	float4 coeffs;
	coeffs.x = g_PI / (2 * sqrt(g_PI));
	coeffs.y = -((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI));
	coeffs.z = ((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI));
	coeffs.w = -((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI));

	coeffs.wyz *= dir;

	return coeffs;
}

static const float3 g_normals[6] = 
{
	float3(1, 0, 0),
	float3(-1, 0, 0),
	float3(0, 1, 0),
	float3(0, -1, 0),
	float3(0, 0, 1),
	float3(0, 0, -1)
};

void main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	uint w, h, d;
	g_colorTexture.GetDimensions(w, h, d);

	uint x = vertexID;
	uint yz = instanceID;
	uint y = yz % w;
	uint z = yz / w;

	int3 voxelPosition = int3(x, y, z);

	//float3 sumColor = 0;
	float3 sumNormal = 0;
	float4 results[3];
	results[0] = 0;
	results[1] = 0;
	results[2] = 0;
	[unroll]
	for (uint i = 0; i < 6; ++i)
	{
		int3 offset = int3(0, 0, i * w);
		float3 color = g_colorTexture.Load(int4(voxelPosition + offset, 0)).rgb;
		float3 normal = g_normalTexture.Load(int4(voxelPosition + offset, 0)).rgb * 2.0 - 1.0;
		normal = normalize(normal);

		float4 coeffs = clampedCosineSHCoeffs(g_normals[i]);

		results[0] += coeffs * color.r;
		results[1] += coeffs * color.g;
		results[2] += coeffs * color.b;
	}

	g_lpvRed[voxelPosition] = results[0];
	g_lpvGreen[voxelPosition] = results[1];
	g_lpvBlue[voxelPosition] = results[2];
}