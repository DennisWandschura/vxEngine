#include "GpuVoxel.h"

struct VSOut
{
	float3 wsPosition : POSITION0;
};

struct GSOutput
{
	float4 pos : SV_POSITION;
	float3 wsPosition : POSITION0;
	uint offset : BLLENDINDICES0;
};

cbuffer VoxelBuffer : register(b0)
{
	GpuVoxel voxel;
};

uint getVoxelDirection(in float3 vertices[3])
{
	float3 e0 = vertices[1] - vertices[0];
	float3 e1 = vertices[2] - vertices[0];

	float3 planeNormal = normalize(cross(e0, e1));
	float3 eyeSpaceNormal = abs(planeNormal);

	float dominantAxis = max(eyeSpaceNormal.x, max(eyeSpaceNormal.y, eyeSpaceNormal.z));

	uint3 cmpAxis;
	cmpAxis.x = (dominantAxis == eyeSpaceNormal.x) ? 0xffffffff : 0;
	cmpAxis.y = (dominantAxis == eyeSpaceNormal.y) ? 0xffffffff : 0;
	cmpAxis.z = (dominantAxis == eyeSpaceNormal.z) ? 0xffffffff : 0;

	uint3 cmpSign;
	cmpSign.x = (planeNormal.x < 0.0) ? 0xffffffff : 0;
	cmpSign.y = (planeNormal.y < 0.0) ? 0xffffffff : 0;
	cmpSign.z = (planeNormal.z < 0.0) ? 0xffffffff : 0;

	uint3 offset = 1;
	offset = offset & cmpSign;

	uint3 indices = uint3(0, 2, 4) + offset;
	indices = indices & cmpAxis;

	return (indices.x + indices.y + indices.z);
}

[maxvertexcount(3)]
void main(
	triangle VSOut input[3],
	inout TriangleStream< GSOutput > output
)
{
	float3 vertices[3];
	vertices[0] = input[0].wsPosition;
	vertices[1] = input[1].wsPosition;
	vertices[2] = input[2].wsPosition;

	uint direction = getVoxelDirection(vertices);

	if (direction == 4 || direction == 5)
	{
		uint zOffset = voxel.dim * (direction == 5);
		for (uint i = 0; i < 3; i++)
		{
			GSOutput element;
			element.pos = mul(voxel.projectionMatrix, float4(input[i].wsPosition, 1.0));
			element.wsPosition = input[i].wsPosition;
			element.offset = zOffset;
			output.Append(element);
		}
	}
}