#ifndef _GPUVOXEL_HH
#define _GPUVOXEL_HH
#ifdef _VX_WINDOWS
#pragma once
#endif

#include "Gpu.h"

struct GpuVoxel
{
	float4x4 projectionMatrix[3];
	int dim;
	int halfDim;
	float invGridCellSize;
	float gridCellSize;
};

#endif