#ifndef _GPU_MATH_HH
#define _GPU_MATH_HH

static const float g_PI = 3.141592654;

float3 quaternionRotation(in float3 v, in float4 q)
{
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

float2 encodeNormal(float3 n)
{
	float f = sqrt(8 * n.z + 8);
	return n.xy / f + 0.5;
}

float3 decodeNormal(float2 enc)
{
	float2 fenc = enc * 4 - 2;
	float f = dot(fenc, fenc);
	float g = sqrt(1 - f / 4);
	float3 n;
	n.xy = fenc*g;
	n.z = 1 - f / 2;
	return n;
}

#endif