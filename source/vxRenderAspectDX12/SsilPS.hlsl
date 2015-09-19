#include "GpuCameraBufferData.h"
#include "gpumath.h"

#define NUM_SAMPLES (9)

static const int tau[] = {
	//  0   1   2   3   4   5   6   7   8   9
	1,  1,  1,  2,  3,  2,  5,  2,  3,  2,  // 0
	3,  3,  5,  5,  3,  4,  7,  5,  5,  7,  // 1
	9,  8,  5,  5,  7,  7,  7,  8,  5,  8,  // 2
	11, 12,  7, 10, 13,  8, 11,  8,  7, 14,  // 3
	11, 11, 13, 12, 13, 19, 17, 13, 11, 18,  // 4
	19, 11, 11, 14, 17, 21, 15, 16, 17, 18,  // 5
	13, 17, 11, 17, 19, 18, 25, 18, 19, 19,  // 6
	29, 21, 19, 27, 31, 29, 21, 18, 17, 29,  // 7
	31, 31, 23, 18, 25, 26, 25, 23, 19, 34,  // 8
	19, 27, 21, 25, 39, 29, 17, 21, 27, 29 }; // 9

#define NUM_SPIRAL_TURNS tau[NUM_SAMPLES * 2]

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

cbuffer CameraStaticBuffer : register(b0)
{
	GpuCameraStatic cameraStatic;
};

Texture2D<float> g_zBuffer : register(t0);
Texture2D<float4> g_directLightning : register(t1);
Texture2D<half2> g_normalSlice : register(t2);

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * cameraStatic.projInfo.xy + cameraStatic.projInfo.zw) * z, -z);
}

float3 getPosition(int2 ssP)
{
	float3 P;

	P.z = g_zBuffer.Load(int3(ssP, 0)).r * cameraStatic.zFar;

	P = reconstructCSPosition(float2(ssP), P.z);

	return P;
}

float3 sampleNormal(in int2 texel, int mip)
{
	float2 packedNormal = g_normalSlice.Load(int3(texel, mip));
	return decodeNormal(packedNormal.rg);
}

float2 tapLocation(int sampleNumber, float spinAngle, float radialJitter, out float ssR)
{
	// Radius relative to ssR
	float alpha = float(sampleNumber + radialJitter) * (1.0 / NUM_SAMPLES);
	float angle = alpha * (NUM_SPIRAL_TURNS * 6.28) + spinAngle;

	ssR = alpha;
	return float2(cos(angle), sin(angle));
}

void sampleDirectLightning(float radius, in int2 texelPos, in float3 position, in float3 normal, inout float3 irradianceSum, inout int numSamplesUsed)
{
	float3 currentPosition = getPosition(texelPos);
	float3 currentNormal = sampleNormal(texelPos, 0);

	float3 YminusX = currentPosition - position;
	float distance = length(YminusX);
	float3 w_i = YminusX / distance;

	float iDotN = clamp(dot(w_i, normal), 0, 1);
	bool cmp0 = (iDotN != 0.0);
	bool cmp1 = (distance <= radius);

	float4 directLight = g_directLightning.Load(int3(texelPos, 0));
	float3 lightIntensity = directLight.rgb * iDotN;
	float lightSum = lightIntensity.x + lightIntensity.y + lightIntensity.z;
	int sampleUsed = (cmp0 && cmp1 && (lightSum != 0.0)) ? 1 : 0;

	irradianceSum += lightIntensity * sampleUsed;

	numSamplesUsed += sampleUsed;
}

void sampleIndirectLight(float radius, float ssDiskRadius, int tapIndex, float randomPatternRotationAngle, float radialJitter, in int2 texel, in float3 position, in float3 normal, inout float3 irradianceSum, inout int numSamplesUsed)
{
	float ssR;
	float2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, radialJitter, ssR);

	ssR *= ssDiskRadius;
	int2 ssP = int2(ssR * unitOffset) + texel;
	sampleDirectLightning(radius, ssP, position, normal, irradianceSum, numSamplesUsed);
}

float4 main(GSOutput input) : SV_TARGET
{
	int2 texel = int2(input.pos.xy);

	float3 vsPosition = getPosition(texel);
	float3 vsNormal = sampleNormal(texel, 0);

	float radius = 1.0;
	float projScale = 500.0;

	float randomPatternRotationAngle = (3 * texel.x ^ texel.y + texel.x * texel.y) * 10;
	float radialJitter = frac(sin(texel.x * 1e2 + texel.y) * 1e5 + sin(texel.y * 1e3) * 1e3) * 0.8 + 0.1;

	float ssDiskRadius = projScale * radius / vsPosition.z;

	int numSamplesUsed = 0;
	float3 irradianceSum = 0;
	for (int i = 0; i < NUM_SAMPLES; ++i)
	{
		sampleIndirectLight(radius, ssDiskRadius, i, randomPatternRotationAngle, radialJitter, texel, vsPosition, vsNormal, irradianceSum, numSamplesUsed);
	}

	const float solidAngleHemisphere = 2 * g_PI;
	float3 E_X = irradianceSum * solidAngleHemisphere / (numSamplesUsed + 0.00001);
	float confidence = float(numSamplesUsed) / float(NUM_SAMPLES);

	return float4(E_X, confidence);
}