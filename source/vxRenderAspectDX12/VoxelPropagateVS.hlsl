#include "gpumath.h"
//#include "gpush.h"

Texture3D<half4> g_srcRed : register(t0);
Texture3D<half4> g_srcGreen : register(t1);
Texture3D<half4> g_srcBlue : register(t2);

RWTexture3D<half4> g_dstRed : register(u0);
RWTexture3D<half4> g_dstGreen : register(u1);
RWTexture3D<half4> g_dstBlue : register(u2);

static const int3 g_offsets[6] =
{
	int3(0, 0, 1),
	int3(1, 0, 0),
	int3(0, 0, -1),
	int3(-1, 0, 0),
	int3(0, 1, 0),
	int3(0, -1, 0)
};

static const float3 g_directions[6] =
{
	float3(0, 0, 1),
	float3(1, 0, 0),
	float3(0, 0, -1),
	float3( -1, 0, 0),
	float3(0, 1, 0),
	float3(0, -1, 0)
};

static float4 g_faceCoeefs[6] =
{
	float4(g_PI / 2.0 * sqrt(g_PI), 0, ((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI)), 0),
	float4(g_PI / 2.0 * sqrt(g_PI), 0, 0, -((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI))),

	float4(g_PI / 2.0 * sqrt(g_PI), 0, -((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI)), 0),
	float4(g_PI / 2.0 * sqrt(g_PI), 0, 0, ((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI))),

	float4(g_PI / 2.0 * sqrt(g_PI), -((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI)), 0, 0),
	float4(g_PI / 2.0 * sqrt(g_PI), ((2.0 * g_PI) / 3.0) * sqrt(3.0 / (4.0 * g_PI)), 0, 0)
};

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

void gather(in int3 gridPosition, in float3 offset, in float3 direction, inout float4 sumRed, inout float4 sumGreen, inout float4 sumBlue, out float3 lumSum)
{
	int3 samplePosition = gridPosition + offset;

	float4 red = g_srcRed.Load(int4(samplePosition, 0));
	float4 green = g_srcGreen.Load(int4(samplePosition, 0));
	float4 blue = g_srcBlue.Load(int4(samplePosition, 0));

	for (int j = 0; j < 6; ++j)
	{
		float3 neighborCellCenter = direction;
		float3 facePosition = g_directions[j] * 0.5;
		float3 dir = facePosition - neighborCellCenter;
		float fLength = length(dir);
		dir /= fLength;

		float solidAngle = 0.0;
		if(fLength > 0.5)
		{
			solidAngle = (fLength >= 1.5) ? (22.95668 / (4 * 180)) : (24.26083 / (4 * 180));
		}

		float4 dirSH;
		dirSH.x = 1.0 / (2 * sqrt(g_PI));
		dirSH.y = -sqrt(3 / (4 * g_PI));
		dirSH.z = sqrt(3 / (4 * g_PI));
		dirSH.w = -sqrt(3 / (4 * g_PI));
		dirSH.wyz *= dir;

		float3 luminance = 0;
		luminance.r = dot(red, dirSH);
		luminance.g = dot(green, dirSH);
		luminance.b = dot(blue, dirSH);
		luminance = max(luminance, 0) * solidAngle;

		//const float amp = 0.9;
		float4 coeffs = g_faceCoeefs[j];
		sumRed += luminance.r * coeffs;// *amp;
		sumGreen += luminance.g * coeffs;// *amp;
		sumBlue += luminance.b * coeffs;//*amp;

		lumSum += luminance;
	}
}

void main(uint xy : SV_VertexID, uint z : SV_InstanceID)
{
	uint w, h, d;
	g_srcRed.GetDimensions(w, h, d);

	uint x = xy & (w - 1);
	uint y = xy / w;

	int3 currentVoxelPosition = int3(x, y, z);
	
	float4 sumRed = g_srcRed.Load(int4(currentVoxelPosition, 0));
	float4 sumGreen = g_srcGreen.Load(int4(currentVoxelPosition, 0));
	float4 sumBlue = g_srcBlue.Load(int4(currentVoxelPosition, 0));

	float3 lumSum = 0;
	gather(currentVoxelPosition, g_offsets[0], g_directions[0], sumRed, sumGreen, sumBlue, lumSum);
	gather(currentVoxelPosition, g_offsets[1], g_directions[1], sumRed, sumGreen, sumBlue, lumSum);
	gather(currentVoxelPosition, g_offsets[2], g_directions[2], sumRed, sumGreen, sumBlue, lumSum);
	gather(currentVoxelPosition, g_offsets[3], g_directions[3], sumRed, sumGreen, sumBlue, lumSum);
	gather(currentVoxelPosition, g_offsets[4], g_directions[4], sumRed, sumGreen, sumBlue, lumSum);
	gather(currentVoxelPosition, g_offsets[5], g_directions[5], sumRed, sumGreen, sumBlue, lumSum);

	g_dstRed[currentVoxelPosition] = sumRed;
	g_dstGreen[currentVoxelPosition] = sumGreen;
	g_dstBlue[currentVoxelPosition] = sumBlue;
}