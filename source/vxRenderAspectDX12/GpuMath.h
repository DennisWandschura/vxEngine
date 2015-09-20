#ifndef _GPU_MATH_HH
#define _GPU_MATH_HH

#define UINT8_MAX 255
#define UINT10_MAX 2047

static const float g_PI = 3.141592654;
static const float3 g_luminanceVector = float3(0.2126, 0.7152, 0.0722);

float3 quaternionRotation(in float3 v, in float4 q)
{
	return v + 2.0 * cross(q.xyz, cross(q.xyz, v) + q.w * v);
}

float signNotZero(in float k) {
	return k >= 0.0 ? 1.0 : -1.0;
}

float2 signNotZero(in float2 v) {
	return float2(signNotZero(v.x), signNotZero(v.y));
}

float2 encodeNormal(float3 n)
{
	float f = sqrt(8 * n.z + 8);
	return n.xy / f + 0.5;
	/*float l1norm = abs(v.x) + abs(v.y) + abs(v.z);
	float2 result = v.xy * (1.0 / l1norm);
	if (v.z < 0.0) {
		result = (1.0 - abs(result.yx)) * signNotZero(result.xy);
	}
	return result;*/
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
	/*float3 v = float3(enc.x, enc.y, 1.0 - abs(enc.x) - abs(enc.y));
	if (v.z < 0) {
		v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
	}
	return normalize(v);*/
}

float getLuminance(in float3 color)
{
	return dot(color, g_luminanceVector);
}

uint packR8G8B8A8(float4 color)
{
	color = clamp(color, 0, 1);
	uint4 colorU8 = uint4(color * 255.0);
	uint packedColor = colorU8.x | (colorU8.y << 8) | (colorU8.z << 16) | (colorU8.w << 24);
	return packedColor;
}

float4 unpackR8G8B8A8(uint packedColor)
{
	uint4 tmp;
	tmp.x = packedColor & 0xff;
	tmp.y = (packedColor >> 8) & 0xff;
	tmp.z = (packedColor >> 16) & 0xff;
	tmp.w = (packedColor >> 24) & 0xff;

	return float4(tmp) * 255.0;
}

#endif