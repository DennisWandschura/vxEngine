#include "GpuSaoBuffer.h"
#include "GpuMath.h"

struct GSOutput
{
	float4 position : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

#define indirectResult  result.rbg
#define visibility      result.a
#define NUM_SAMPLES (17)

// tau[N-1] = optimal number of spiral turns for N samples 2 
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
/*static const int tau[] = { 1, 1, 2, 3, 2, 5, 2, 3, 2, 3, 3, 5, 5, 3, 4, 7, 5, 5, 7, 9, 8, 5, 5, 7, 7, 7, 8, 5, 8, 11, 12, 7, 10, 13, 8, 11, 8, 7,
14, 11, 11, 13, 12, 13, 19, 17, 13, 11, 18, 19, 11, 11, 14, 17, 21, 15, 16, 17, 18, 13, 17, 11, 17, 19, 18, 25, 18, 19, 19, 29, 21, 19, 27,
31, 29, 21, 18, 17, 29, 31, 31, 23, 18, 25, 26, 25, 23, 19, 34, 19, 27, 21, 25, 39, 29, 17, 21, 27 };*/

#define NUM_SPIRAL_TURNS tau[NUM_SAMPLES * 2]

Texture2D CS_Z_buffer: register(t0);
Texture2D g_bounceTexture: register(t1);
Texture2DArray g_normalTexture : register(t2);

cbuffer SaoBufferBlock : register(b0)
{
	GpuSaoBuffer saoBuffer;
};

float3 reconstructCSPosition(float2 S, float z)
{
	return float3((S.xy * saoBuffer.projInfo.xy + saoBuffer.projInfo.zw) * z, z);
}

float sampleZBuffer(int3 ssP)
{
	return CS_Z_buffer.Load(ssP).r * saoBuffer.zFar;
}

float3 getPosition(int2 ssP)
{
	float3 P;

	P.z = sampleZBuffer(int3(ssP, 0));

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP), P.z);// *float3(-1, 1, -1);
	return P;
}

float2 tapLocation(int sampleNumber, float spinAngle, float radialJitter, out float ssR) 
{
	// Radius relative to ssR
	float alpha = float(sampleNumber + radialJitter) * (1.0 / NUM_SAMPLES);
	float angle = alpha * (NUM_SPIRAL_TURNS * 6.28) + spinAngle;

	ssR = alpha;
	return float2(cos(angle), sin(angle));
}

float3 sampleNormal(int2 texel, int miplevel)
{
	float2 compressedNormal = g_normalTexture.Load(int4(texel, 0, miplevel)).rg;
	return normalize(decodeNormal(compressedNormal))*float3(-1, 1, -1);
}

float3 reconstructNonUnitCSFaceNormal(float3 C)
{
	//return cross(ddy(C), ddx(C));
	return cross(ddy_fine(C), ddx_fine(C));
}

float3 reconstructCSFaceNormal(float3 C)
{
	return normalize(reconstructNonUnitCSFaceNormal(C));
}

void getOffsetPositionNormalAndLambertian
(int2            ssP,
	float            ssR,
	out float3       Q,
	out float3       lambertian_tap,
	out float3      n_tap) 
{

/*#   if USE_MIPMAPS
	int mipLevel;
	ivec2 texel;
	computeMipInfo(ssR, ssP, cszBuffer, mipLevel, texel);
#   else*/
	int mipLevel = 0;
	int2 texel = ssP;

	float z = sampleZBuffer(int3(texel, mipLevel));//texelFetch(cszBuffer, texel, mipLevel).r;
	float3 n = sampleNormal(texel, mipLevel);
	lambertian_tap = g_bounceTexture.Load(int3(texel, mipLevel)).rgb;//texelFetch(bounceBuffer, texel, mipLevel).rgb;

	// Offset to pixel center
	Q = reconstructCSPosition((float2(ssP)+float2(0.5, 0.5)), z);
	//float3 n = reconstructCSFaceNormal(Q);
	n_tap = n;
}

void iiValueFromPositionsAndNormalsAndLambertian(int2 ssP, float3 X, float3 n_X, float3 Y, float3 n_Y, float3 radiosity_Y, float radius2, out float3 E, out float weight_Y) 
{
	float3 YminusX = Y - X;
	float3 w_i = normalize(YminusX);

	float iDotN = dot(w_i, n_X);

	weight_Y = ((iDotN > 0.0) && (dot(-w_i, n_Y) > 0.01)) ? 1.0 : 0.0; // Backface check

	if ((dot(YminusX, YminusX) < radius2) && // Radius check
		(weight_Y > 0)) 
	{
		E = radiosity_Y * iDotN;
	}
	else 
	{
		E = 0;
	}
}

void sampleIndirectLight
(in int2            ssC,
	in float3           C,
	in float3          n_C,
	//in float3           C_peeled,
	//in float3          n_C_peeled,
	in float            ssDiskRadius,
	in int              tapIndex,
	in float            randomPatternRotationAngle,
	in float            radialJitter,
	float radius2,
	inout float3   irradianceSum,
	inout float         numSamplesUsed,
	inout float3   iiPeeled,
	inout float         weightSumPeeled) {

	// Not used yet, quality optimization in progress...
	//float visibilityWeightPeeled0, visibilityWeightPeeled1;

	// Offset on the unit disk, spun for this pixel
	float ssR;
	float2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, radialJitter, ssR);
	ssR *= ssDiskRadius;
	int2 ssP = int2(ssR * unitOffset) + ssC;

	float3 E;
	//float visibilityWeight;
	float weight_Y;
	// The occluding point in camera space
	float3 Q, lambertian_tap, n_tap;
	getOffsetPositionNormalAndLambertian(ssP, ssR, Q, lambertian_tap, n_tap);
	iiValueFromPositionsAndNormalsAndLambertian(ssP, C, n_C, Q, n_tap, lambertian_tap, radius2, E, weight_Y);
	numSamplesUsed += weight_Y;

	irradianceSum += E;
}

float4 main(GSOutput input) : SV_TARGET
{
	float4 result = 0;

	// Pixel being shaded 
	int2 ssC = int2(input.position.xy);
	float radius2 = saoBuffer.radius * saoBuffer.radius;

	float3 C = getPosition(ssC);
	//float3 C_peeled = 0;
	//float3 n_C_peeled = 0;

	//float3 n_C = sampleNormal(normal_buffer, ssC, 0);
	float3 n_C = sampleNormal(ssC, 0);
	//float3 n_C = reconstructCSFaceNormal(C);

	// Choose the screen-space sample radius
	// proportional to the projected area of the sphere
	float ssDiskRadius = -saoBuffer.projScale * saoBuffer.radius / C.z;
	//ssDiskRadius = abs(ssDiskRadius);

	// Hash function used in the HPG12 AlchemyAO paper
	float randomPatternRotationAngle = (3 * ssC.x ^ ssC.y + ssC.x * ssC.y) * 10;

	float radialJitter = frac(sin(input.position.x * 1e2 + input.position.y) * 1e5 + sin(input.position.y * 1e3) * 1e3) * 0.8 + 0.1;

	float numSamplesUsed = 0.0;
	float3 irradianceSum = 0;
	float3 ii_peeled = 0;
	float peeledSum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) 
	{
		sampleIndirectLight(ssC, C, n_C, ssDiskRadius, i, randomPatternRotationAngle, radialJitter, radius2, irradianceSum, numSamplesUsed, ii_peeled, peeledSum);
	}

	const float solidAngleHemisphere = 2 * g_PI;
	float3 E_X = irradianceSum * solidAngleHemisphere / (numSamplesUsed + 0.00001);
	E_X = clamp(E_X, 0, 1);

	indirectResult = E_X;

// What is the ambient visibility of this location
	visibility = 1 - numSamplesUsed / float(NUM_SAMPLES);

	return result;
}