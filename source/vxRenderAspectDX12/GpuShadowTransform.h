#include "Gpu.h"

struct ShadowTransform
{
	float4x4 projectionMatrix;
	float4x4 pvMatrix[6];
};