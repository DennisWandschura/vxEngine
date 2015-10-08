#ifdef _VX_WINDOWS
#pragma once
#endif

#ifndef _GPUVOXEL_HH
#define _GPUVOXEL_HH
#include "Gpu.h"

struct GpuVoxel
{
	float4x4 projectionMatrix[3];
	float4 gridCenter;
//	float4 prevGridCenter;
	int dim;
	int halfDim;
	float invGridCellSize;
	float gridCellSize;
};

#endif