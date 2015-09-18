/**
\file SAO_AO.pix
\author Morgan McGuire and Michael Mara, NVIDIA Research

Reference implementation of the Scalable Ambient Obscurance (SAO) screen-space ambient obscurance algorithm.

The optimized algorithmic structure of SAO was published in McGuire, Mara, and Luebke, Scalable Ambient Obscurance,
<i>HPG</i> 2012, and was developed at NVIDIA with support from Louis Bavoil.

The mathematical ideas of AlchemyAO were first described in McGuire, Osman, Bukowski, and Hennessy, The
Alchemy Screen-Space Ambient Obscurance Algorithm, <i>HPG</i> 2011 and were developed at
Vicarious Visions.

DX11 HLSL port by Leonardo Zide of Treyarch

<hr>

Open Source under the "BSD" license: http://www.opensource.org/licenses/bsd-license.php

Copyright (c) 2011-2012, NVIDIA
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include "GpuSaoBuffer.h"
#include "GpuMath.h"

#define visibility      fragment.color.r
#define bilateralKey    fragment.color.gb

/** Used for preventing AO computation on the sky (at infinite depth) and defining the CS Z to bilateral depth key scaling.
This need not match the real far plane*/
#define FAR_PLANE_Z (300.0)

// Total number of direct samples to take at each pixel
#define NUM_SAMPLES (14)

// This is the number of turns around the circle that the spiral pattern makes.  This should be prime to prevent
// taps from lining up.  This particular choice was tuned for NUM_SAMPLES == 9
//#define NUM_SPIRAL_TURNS (7)

// If using depth mip levels, the log of the maximum pixel offset before we need to switch to a lower 
// miplevel to maintain reasonable spatial locality in the cache
// If this number is too small (< 3), too many taps will land in the same pixel, and we'll get bad variance that manifests as flashing.
// If it is too high (> 5), we'll get bad performance because we're not using the MIP levels effectively
#define LOG_MAX_OFFSET 3

// This must be less than or equal to the MAX_MIP_LEVEL defined in SSAO.cpp
#define MAX_MIP_LEVEL 5

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

struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 texCoords : TEXCOORD0;
};

struct PixelOutput
{
	float4 color : SV_TARGET0;
};

Texture2D<float> CS_Z_buffer : register(t0);
Texture2D<half2> g_normalTexture : register(t1);

cbuffer SaoBufferBlock : register(b0)
{
	GpuSaoBuffer saoBuffer;
};

/** Used for packing Z into the GB channels */
float CSZToKey(float z)
{
	return clamp(z * (1.0 / FAR_PLANE_Z), 0.0, 1.0);
}

/** Used for packing Z into the GB channels */
void packKey(float key, out float2 p)
{
	// Round to the nearest 1/256.0
	float temp = floor(key * 256.0);

	// Integer part
	p.x = temp * (1.0 / 256.0);

	// Fractional part
	p.y = key * 256.0 - temp;
}

/** Reconstruct camera-space P.xyz from screen-space S = (x, y) in
pixels and camera-space z < 0.  Assumes that the upper-left pixel center
is at (0.5, 0.5) [but that need not be the location at which the sample tap
was placed!]

Costs 3 MADD.  Error is on the order of 10^3 at the far plane, partly due to z precision.
*/
float3 reconstructCSPosition(float2 S, float z)
{
	float3 p = float3((S.xy * saoBuffer.projInfo.xy + saoBuffer.projInfo.zw) * z, z);
	return p;
}

float sampleZBuffer(int3 ssP)
{
	return CS_Z_buffer.Load(ssP).r * saoBuffer.zFar;
}

/** Read the camera-space position of the point at screen-space pixel ssP */
float3 getPosition(int2 ssP)
{
	float3 P;

	P.z = sampleZBuffer(int3(ssP, 0));

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP), P.z);
	return P;
}

float3 reconstructNonUnitCSFaceNormal(float3 C)
{
	//return cross(ddy(C), ddx(C));
	return cross(ddy_fine(C), ddx_fine(C));
}

/** Reconstructs screen-space unit normal from screen-space position */
float3 reconstructCSFaceNormal(float3 C)
{
	return normalize(reconstructNonUnitCSFaceNormal(C));
}

/** Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
float2 tapLocation(int sampleNumber, float spinAngle, out float ssR)
{
	// Radius relative to ssR
	float alpha = float(sampleNumber + 0.5) * (1.0 / NUM_SAMPLES);
	float angle = alpha * (tau[2 * NUM_SAMPLES] * 6.28) + spinAngle;

	ssR = alpha;
	return float2(cos(angle), sin(angle));
}

/** Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1 */
/*void getOffsetPositions(int2 ssC, float2 unitOffset, float ssR, float radius2, out float3 P0, out float3 P1)
{
	// Derivation:
	//  mipLevel = floor(log(ssR / MAX_OFFSET));
	int mipLevel = clamp((int)floor(log2(ssR)) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	int2 ssP = int2(ssR*unitOffset) + ssC;

	// Divide coordinate by 2^mipLevel
	P0.z = CS_Z_buffer.Load(int3(ssP >> mipLevel, mipLevel)).r;
	P1.z = CS_Z_buffer1.Load(int3(ssP >> mipLevel, mipLevel)).r;

	// Offset to pixel center
	P0 = reconstructCSPosition(float2(ssP), P0.z);
	P1 = reconstructCSPosition(float2(ssP), P1.z);
}*/

float3 getOffsetPosition(int2 ssC, float2 unitOffset, float ssR)
{
	// Derivation:
	//  mipLevel = floor(log(ssR / MAX_OFFSET));
	int mipLevel = clamp((int)floor(log2(ssR)) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	int2 ssP = int2(ssR * unitOffset) + ssC;

	float3 P;

	// Divide coordinate by 2^mipLevel
	//P.z = CS_Z_buffer.Load(int3(ssP >> mipLevel, mipLevel)).r;
	P.z = sampleZBuffer(int3(ssP >> mipLevel, mipLevel));

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP), P.z);
	return P;
}

float fallOffFunction(float vv, float vn, float epsilon, float invRadius2)
{
	// A: From the HPG12 paper
	// Note large epsilon to avoid overdarkening within cracks
	// return float(vv < radius2) * max((vn - bias) / (epsilon + vv), 0.0) * radius2 * 0.6;

	// B: Smoother transition to zero (lowers contrast, smoothing out corners). [Recommended]
	//float f = max(radius2 - vv, 0.0);
	//return f * f * f * max((vn - saoBuffer.bias) / (epsilon + vv), 0.0);

	float f = max(1.0 - vv * invRadius2, 0.0);
	return f * max((vn - saoBuffer.bias) * rsqrt(epsilon + vv), 0.0);

	// C: Medium contrast (which looks better at high radii), no division.  Note that the 
	// contribution still falls off with radius^2, but we've adjusted the rate in a way that is
	// more computationally efficient and happens to be aesthetically pleasing.
	//float invRadius2 = 1.0 / radius2;
	//return 4.0 * max(1.0 - vv * invRadius2, 0.0) * max(vn - saoBuffer.bias, 0.0);

	// D: Low contrast, no division operation
	// return 2.0 * float(vv < radius * radius) * max(vn - bias, 0.0);
}

/** Compute the occlusion due to sample point \a Q about camera-space point \a C with unit normal \a n_C */
float aoValueFromPositionsAndNormal(float3 C, float3 n_C, float3 Q, float invRadius2)
{
	float3 v = Q - C;
	float vv = dot(v, v);
	float vn = dot(v, n_C);
	const float epsilon = 0.001;

	// Without the angular adjustment term, surfaces seen head on have less AO
	return fallOffFunction(vv, vn, epsilon, invRadius2) * lerp(1.0, max(0.0, 1.5 * n_C.z), 0.35);
}

/** Compute the occlusion due to sample with index \a i about the pixel at \a ssC that corresponds
to camera-space point \a C with unit normal \a n_C, using maximum screen-space sampling radius \a ssDiskRadius */
float sampleAO(in int2 ssC, in float3 C, in float3 n_C, in float ssDiskRadius, in int tapIndex, in float randomPatternRotationAngle, float radius2, float invRadius2)
{
	// Offset on the unit disk, spun for this pixel
	float ssR;
	float2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, ssR);
	// Ensure that the taps are at least 1 pixel away
	//ssR = max(0.75, ssR * ssDiskRadius);
	ssR *= ssDiskRadius;

	// The occluding point in camera space
	float3 Q = getOffsetPosition(ssC, unitOffset, ssR);//*float3(-1, 1, -1);;
	return aoValueFromPositionsAndNormal(C, n_C, Q, invRadius2);
	/*float3 Q0;
	float3 Q1;
	getOffsetPositions(ssC, unitOffset, ssR, radius2, Q0, Q1);

	return max(aoValueFromPositionsAndNormal(C,  n_C, Q0, invRadius2), aoValueFromPositionsAndNormal(C, n_C, Q1, invRadius2));*/
}

float square(float x)
{
	return x * x;
}

PixelOutput main(GSOutput input)
{
	const float MIN_RADIUS = 3.0; // pixels

	PixelOutput fragment;
	fragment.color = 1;

	// Pixel being shaded 
	int2 ssC = int2(input.pos.xy);

	// World space point being shaded
	float3 C = getPosition(ssC);//*float3(-1, 1, -1);

	packKey(CSZToKey(C.z), bilateralKey);

	// Hash function used in the HPG12 AlchemyAO paper
	float randomPatternRotationAngle = (((3 * ssC.x) ^ (ssC.y + ssC.x * ssC.y)));

	// Reconstruct normals from positions. These will lead to 1-pixel black lines
	// at depth discontinuities, however the blur will wipe those out so they are not visible
	// in the final image.
	//float3 n_C = reconstructCSFaceNormal(C)*float3(-1, 1, -1);
	/*float3 n_C = reconstructNonUnitCSFaceNormal(C);
	if (dot(n_C, n_C) > (square(C.z * C.z * 0.00006)))
	{
		// if the threshold # is too big you will see black dots where we used a bad normal at edges, too small -> white
		// The normals from depth should be very small values before normalization,
		// except at depth discontinuities, where they will be large and lead
		// to 1-pixel false occlusions because they are not reliable
		visibility = 1.0;
		return fragment;
	}
	else
	{
		n_C = normalize(n_C);
	}*/

	float2 compressedNormal = g_normalTexture.Load(int3(ssC, 0)).rg;
	float3 n_C = normalize(decodeNormal(compressedNormal)) * float3(-1, 1, -1);

	// Choose the screen-space sample radius
	// proportional to the projected area of the sphere
	float ssDiskRadius = -saoBuffer.projScale * saoBuffer.radius / C.z;
	
	/*if (ssDiskRadius <= MIN_RADIUS)
	{
		// There is no way to compute AO at this radius
		visibility = 1.0;
		return fragment;
	}*/

	float radius2 = saoBuffer.radius * saoBuffer.radius;
	float invRadius2 = 1.0 / radius2;

	float sum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i)
	{
		sum += sampleAO(ssC, C, n_C, ssDiskRadius, i, randomPatternRotationAngle, radius2, invRadius2);
	}

	float intensityDivR6 = saoBuffer.intensity / pow(saoBuffer.radius, 6.0f);
	float A = max(0.0, 1.0 - sum * intensityDivR6 * (5.0 / NUM_SAMPLES));

	//float A = pow(max(0.0, 1.0 - sqrt(sum * (3.0 / NUM_SAMPLES))), saoBuffer.intensity);
	A = lerp(A, 1.0f, 1.0f - saturate(0.5f * C.z));

	// Bilateral box-filter over a quad for free, respecting depth edges
	// (the difference that this makes is subtle)
	/*if (abs(ddx(C.z)) < 0.02) {
		A -= ddx(A) * ((ssC.x & 1) - 0.5);
	}
	if (abs(ddy(C.z)) < 0.02) {
		A -= ddy(A) * ((ssC.y & 1) - 0.5);
	}*/

	visibility = A;
	//visibility = lerp(1.0, A, saturate(MIN_RADIUS - ssDiskRadius));

	return fragment;
}