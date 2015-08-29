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

#define visibility      fragment.color.r
#define bilateralKey    fragment.color.gb

/** Used for preventing AO computation on the sky (at infinite depth) and defining the CS Z to bilateral depth key scaling.
This need not match the real far plane*/
#define FAR_PLANE_Z (200.0)

// Total number of direct samples to take at each pixel
#define NUM_SAMPLES (9)

// This is the number of turns around the circle that the spiral pattern makes.  This should be prime to prevent
// taps from lining up.  This particular choice was tuned for NUM_SAMPLES == 9
#define NUM_SPIRAL_TURNS (7)

// If using depth mip levels, the log of the maximum pixel offset before we need to switch to a lower 
// miplevel to maintain reasonable spatial locality in the cache
// If this number is too small (< 3), too many taps will land in the same pixel, and we'll get bad variance that manifests as flashing.
// If it is too high (> 5), we'll get bad performance because we're not using the MIP levels effectively
#define LOG_MAX_OFFSET 3

// This must be less than or equal to the MAX_MIP_LEVEL defined in SSAO.cpp
#define MAX_MIP_LEVEL 5

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
	return float3((S.xy * saoBuffer.projInfo.xy + saoBuffer.projInfo.zw) * z, z);
}

/** Read the camera-space position of the point at screen-space pixel ssP */
float3 getPosition(int2 ssP)
{
	float3 P;

	P.z = CS_Z_buffer.Load(int3(ssP, 0)).r;

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP) + float2(0.5, 0.5), P.z);
	return P;
}

/** Reconstructs screen-space unit normal from screen-space position */
float3 reconstructCSFaceNormal(float3 C) 
{
	return normalize(cross(ddy(C), ddx(C)));
}

/** Returns a unit vector and a screen-space radius for the tap on a unit disk (the caller should scale by the actual disk radius) */
float2 tapLocation(int sampleNumber, float spinAngle, out float ssR) 
{
	// Radius relative to ssR
	float alpha = float(sampleNumber + 0.5) * (1.0 / NUM_SAMPLES);
	float angle = alpha * (NUM_SPIRAL_TURNS * 6.28) + spinAngle;

	ssR = alpha;
	return float2(cos(angle), sin(angle));
}

/** Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1 */
float3 getOffsetPosition(int2 ssC, float2 unitOffset, float ssR, float radius2)
{
	// Derivation:
	//  mipLevel = floor(log(ssR / MAX_OFFSET));
	int mipLevel = clamp((int)floor(log2(ssR)) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	int2 ssP = int2(ssR*unitOffset) + ssC;

	float3 P;

	// Divide coordinate by 2^mipLevel
	P.z = CS_Z_buffer.Load(int3(ssP >> mipLevel, mipLevel)).r;

	// Offset to pixel center
	P = reconstructCSPosition(float2(ssP)+float2(0.5, 0.5), P.z);

	return P;
}

/** Compute the occlusion due to sample with index \a i about the pixel at \a ssC that corresponds
to camera-space point \a C with unit normal \a n_C, using maximum screen-space sampling radius \a ssDiskRadius */
float sampleAO(in int2 ssC, in float3 C, in float3 n_C, in float ssDiskRadius, in int tapIndex, in float randomPatternRotationAngle, float radius2) 
{
	// Offset on the unit disk, spun for this pixel
	float ssR;
	float2 unitOffset = tapLocation(tapIndex, randomPatternRotationAngle, ssR);
	ssR *= ssDiskRadius;

	// The occluding point in camera space
	float3 Q = getOffsetPosition(ssC, unitOffset, ssR, radius2);

	float3 v = Q - C;

	float vv = dot(v, v);
	float vn = dot(v, n_C);

	const float epsilon = 0.01;

	// A: From the HPG12 paper
	// Note large epsilon to avoid overdarkening within cracks
	// return float(vv < radius2) * max((vn - bias) / (epsilon + vv), 0.0) * radius2 * 0.6;

	// B: Smoother transition to zero (lowers contrast, smoothing out corners). [Recommended]
	float f = max(radius2 - vv, 0.0); 
	return f * f * f * max((vn - saoBuffer.bias) / (epsilon + vv), 0.0);

	// C: Medium contrast (which looks better at high radii), no division.  Note that the 
	// contribution still falls off with radius^2, but we've adjusted the rate in a way that is
	// more computationally efficient and happens to be aesthetically pleasing.
	//float invRadius2 = 1.0 / radius2;
	//return 4.0 * max(1.0 - vv * invRadius2, 0.0) * max(vn - saoBuffer.bias, 0.0);

	// D: Low contrast, no division operation
	// return 2.0 * float(vv < radius * radius) * max(vn - bias, 0.0);
}

PixelOutput main(GSOutput input)
{
	PixelOutput fragment;
	fragment.color = 1;

	// Pixel being shaded 
	int2 ssC = int2(input.pos.xy);

	// World space point being shaded
	float3 C = getPosition(ssC);

	packKey(CSZToKey(C.z), bilateralKey);

	// Hash function used in the HPG12 AlchemyAO paper
	float randomPatternRotationAngle = (3 * ssC.x ^ ssC.y + ssC.x * ssC.y) * 10;

	// Reconstruct normals from positions. These will lead to 1-pixel black lines
	// at depth discontinuities, however the blur will wipe those out so they are not visible
	// in the final image.
	float3 n_C = reconstructCSFaceNormal(C);

	// Choose the screen-space sample radius
	// proportional to the projected area of the sphere
	float ssDiskRadius = -saoBuffer.projScale * saoBuffer.radius / C.z;
	float radius2 = saoBuffer.radius * saoBuffer.radius;

	float sum = 0.0;
	for (int i = 0; i < NUM_SAMPLES; ++i) 
	{
		sum += sampleAO(ssC, C, n_C, ssDiskRadius, i, randomPatternRotationAngle, radius2);
	}

	float intensityDivR6 = saoBuffer.intensity / pow(saoBuffer.radius, 6.0f);

	float A = max(0.0, 1.0 - sum * intensityDivR6 * (5.0 / NUM_SAMPLES));
	A= lerp(A, 1.0f, 1.0f - saturate(0.5f * C.z));

	// Bilateral box-filter over a quad for free, respecting depth edges
	// (the difference that this makes is subtle)
	if (abs(ddx(C.z)) < 0.02) {
		A -= ddx(A) * ((ssC.x & 1) - 0.5);
	}
	if (abs(ddy(C.z)) < 0.02) {
		A -= ddy(A) * ((ssC.y & 1) - 0.5);
	}

	visibility = A;

	return fragment;
}