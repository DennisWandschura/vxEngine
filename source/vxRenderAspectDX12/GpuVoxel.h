#ifndef _GPUVOXEL_HH
#define _GPUVOXEL_HH
#ifdef _VX_WINDOWS
#pragma once
#include "Gpu.h"
#endif

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