#include "Gpu.h"

struct GpuVoxel
{
	float4x4 projectionMatrix;
	int dim;
	int halfDim;
	float invGridCellSize;
	float gridCellSize;
};