#include "gpulpv.h"
#include "GpuShadowTransform.h"

struct VSOutput
{
	float4 position : SV_POSITION;
	uint zSlice : BLENDINDICES0;
};

cbuffer GpuLpvBuffer
{
	GpuLpv g_lpv;
};

Texture2DArray<float4> g_color;
Texture2DArray<half4> g_normals;
Texture2DArray<float> g_depth;

StructuredBuffer<GpuShadowTransformReverse> g_transforms : register(t3);

static const float3 lightDirections[6] =
{
	float3(1, 0, 0),
	float3(-1, 0, 0),
	float3(0, 1, 0),
	float3(0, -1, 0),
	float3(0, 0, -1),
	float3(0, 0, 1)
};

bool isInGrid(int3 position)
{
	bool result = true;
	if (position.x < 0 || position.y < 0 || position.z < 0 ||
		position.x >= g_lpv.dim || position.y >= g_lpv.dim || position.z >= g_lpv.dim)
		result = false;

	return result;
}

half4 SHRotate(const in half3 vcDir, const in half2 vZHCoeffs) 
{
	// compute sine and cosine of thetta angle   
	// beware of singularity when both x and y are 0 (no need to rotate at all)
	half2 theta12_cs = normalize(vcDir.xy);
	// compute sine and cosine of phi angle  
	half2 phi12_cs;
	phi12_cs.x = sqrt(1.h - vcDir.z * vcDir.z);
	phi12_cs.y = vcDir.z;  
	half4 vResult;   // The first band is rotation-independent
	vResult.x =  vZHCoeffs.x;   // rotating the second band of SH
	vResult.y =  vZHCoeffs.y * phi12_cs.x * theta12_cs.y;
	vResult.z = -vZHCoeffs.y * phi12_cs.y;
	vResult.w =  vZHCoeffs.y * phi12_cs.x * theta12_cs.x;
	return vResult;
}

half4 SHProjectCone(const in half3 vcDir, uniform half angle) 
{
	const half2 vZHCoeffs = half2(
		.5h * (1.h - cos(angle)), // 1/2 (1 - Cos[\[Alpha]])
		0.75h * sin(angle) * sin(angle)); // 3/4 Sin[\[Alpha]]^2
	return SHRotate(vcDir, vZHCoeffs); 
}

half4 SHProjectCone(const in half3 vcDir) 
{
	static const half2 vZHCoeffs = half2(.25h,   // 1/4
		.5h);   // 1/2
	return SHRotate(vcDir, vZHCoeffs); 
}

VSOutput main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
	int2 texel = int2(vertexID, instanceID);
	int arrayIndex = 0;
	int lightIndex = 0;
	int cubeIndex = 0;

	uint w, h, d;
	g_depth.GetDimensions(w, h, d);
	float2 texCoord = float2(texel) / float2(w, h);

	float depth = g_depth.Load(int4(texel, arrayIndex, 0));
	half2 screenPos = texCoord * float2(2, -2) - float2(1, -1);

	float4x4 invPvMatrix = g_transforms[lightIndex].invPvMatrix[cubeIndex];

	float4 wsPosition = mul(invPvMatrix, float4(screenPos, depth, 1.f));

	half3 normal = g_normals.Load(int4(texel, arrayIndex, 0)).xyz;

	// shift by half cell size in normal direction
	wsPosition.xyz += normal * g_lpv.gridCellSize * 0.5;
	// shift by half cell size in light direction
	wsPosition.xyz += -lightDirections[cubeIndex] * g_lpv.gridCellSize * 0.5;

	float3 offset = (wsPosition.xyz - g_lpv.center.xyz) * g_lpv.invGridCellSize;
	int3 gridPosition = int3(offset) + g_lpv.halfDim;

	float3 alignedGridPosition = float3(gridPosition);

	//alignedGridPosition += normal * 0.5;
	//alignedGridPosition.z = floor(alignedGridPosition.z * g_lpv.dim) / g_lpv.dim;

	int rsmTexelCount = w * h;
	int lpvCellCount = g_lpv.dim * g_lpv.dim * g_lpv.dim;
	float surfelWeight = float(rsmTexelCount) / float(lpvCellCount);

	float3 lightColor = g_color.Load(int4(texel, arrayIndex, 0)).xyz;
	float3 lightColorWeighted = lightColor * surfelWeight;

	half4 sHCoeffs = SHProjectCone(normal);
	float4 shRed = sHCoeffs * float4(lightColor.rrrr);
	float4 shGreen = sHCoeffs * float4(lightColor.gggg);
	float4 shBlue = sHCoeffs * float4(lightColor.bbbb);

	VSOutput output;

	// 0...1
	output.position.xy = float2(alignedGridPosition.xy / float(g_lpv.dim));
	output.position.xy = output.position.xy * float2(2, -2) - float2(1, -1);
	output.position.z = 0;
	output.position.w = 1;
	output.zSlice = gridPosition.z;

	bool inGrid = isInGrid(gridPosition);
	if (!inGrid)
	{
		output.position.xy = -2;
	}

	return output;
}