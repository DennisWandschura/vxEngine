#pragma once

#ifndef _GPULPV_HH
#define _GPULPV_HH
#ifdef _VX_WINDOWS
#endif

#include "Gpu.h"

struct GpuLpv
{
	int dim;
	int halfDim;
	float invGridCellSize;
	float gridCellSize;
	float4 center;
};

#endif