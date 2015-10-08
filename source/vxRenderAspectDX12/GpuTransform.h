#ifdef _VX_WINDOWS
#pragma once
#endif

#ifndef _GPU_TRANSFORM_HH
#define _GPU_TRANSFORM_HH
#include "Gpu.h"

struct TransformGpu
{
	float3 translation;
	float scaling;
	uint2 packedQRotation;
	float padding[2];
};

#if _VX_WINDOWS
#include <vxEngineLib/Transform.h>
static_assert(sizeof(vx::TransformGpu) == sizeof(TransformGpu), "wrong size");
#else
float4 unpackQRotation(in uint2 rotation)
{
	uint4 tmp;
	tmp.x = rotation.x & 0xffff;
	tmp.y = (rotation.x >> 16) & 0xffff;
	tmp.z = rotation.y & 0xffff;
	tmp.w = (rotation.y >> 16) & 0xffff;

	const float4 vmax = { 0xffff, 0xffff, 0xffff, 0xffff };

	float4 qRotation;
	qRotation.x = (float)tmp.x;
	qRotation.y = (float)tmp.y;
	qRotation.z = (float)tmp.z;
	qRotation.w = (float)tmp.w;

	qRotation = qRotation / vmax;
	qRotation = qRotation * float4(2, 2, 2, 2);
	qRotation = qRotation - float4(1, 1, 1, 1);

	return qRotation;
}
#endif
#endif