#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"

struct VSOutput
{
	int3 xyz : POSITION0;
	uint axis : AXIS0;
	float4 color : COLOR0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float4 color : SV_TARGET0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

cbuffer CameraBuffer : register(b1)
{
	GpuCameraBufferData camera;
};

[maxvertexcount(3)]
void main(
	point VSOutput input[1],
	inout TriangleStream< GSOutput > output
)
{
	int3 voxelPos = input[0].xyz;

	if (input[0].color.a != 0)
	{
		float3 voxelCenter = voxel.gridCenter.xyz;
		float3 offset = voxel.halfDim;

		float l = voxelPos.x - offset.x;
		float r = voxelPos.x + 1 - offset.x;

		float b = voxelPos.y - offset.y;
		float t = voxelPos.y + 1 - offset.y;

		float f = voxelPos.z - offset.z;
		float n = voxelPos.z + 1 - offset.z;


		// axis 0
		// rbn, rbf, rtf, rtn

		// axis1
		// lbf, lbn, ltn,, ltf
	}
}