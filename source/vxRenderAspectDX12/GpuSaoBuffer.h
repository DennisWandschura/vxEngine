#ifdef _VX_WINDOWS
#pragma once
#endif

#ifndef _GPUSAOBUFFER_HH
#define _GPUSAOBUFFER_HH
#include "Gpu.h"

struct GpuSaoBuffer
{
	float4 projInfo;
	float bias;
	float intensity;
	float projScale;
	float radius;
	float zFar;
};
#endif