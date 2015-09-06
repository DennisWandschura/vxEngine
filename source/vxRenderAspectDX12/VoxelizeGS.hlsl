#include "GpuVoxel.h"
#include "GpuCameraBufferData.h"

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

cbuffer CameraBuffer : register(b1)
{
	GpuCameraBufferData camera;
};

uint getVoxelDirection(in float3 vertices[3], out uint matrixIndex, out uint textureOffset)
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
	textureOffset = (indices.x + indices.y + indices.z) * voxel.dim;

	uint3 matrixIndices = uint3(0, 1, 2) & cmpAxis;
	matrixIndex = matrixIndices.x + matrixIndices.y + matrixIndices.z;

	return (indices.x + indices.y + indices.z);
}

[maxvertexcount(3)]
void main(
	triangle VSOut input[3],
	inout TriangleStream< GSOutput > output
)
{
	float3 cameraPosition = camera.position.xyz;

	float3 voxelCenter = cameraPosition * voxel.invGridCellSize;
	voxelCenter = float3(int3(voxelCenter)) * voxel.gridCellSize;

	float3 vertices[3];
	vertices[0] = input[0].wsPosition;
	vertices[1] = input[1].wsPosition;
	vertices[2] = input[2].wsPosition;

	vertices[0] -= voxelCenter;
	vertices[1] -= voxelCenter;
	vertices[2] -= voxelCenter;

	uint matrixIndex = 0;
	uint textureOffset = 0;
	uint direction = getVoxelDirection(vertices, matrixIndex, textureOffset);

	GSOutput element;
	element.offset = textureOffset;

	element.pos = mul(voxel.projectionMatrix[matrixIndex], float4(vertices[0], 1.0));
	element.wsPosition = vertices[0];
	output.Append(element);

	element.pos = mul(voxel.projectionMatrix[matrixIndex], float4(vertices[1], 1.0));
	element.wsPosition = vertices[1];
	output.Append(element);

	element.pos = mul(voxel.projectionMatrix[matrixIndex], float4(vertices[2], 1.0));
	element.wsPosition = vertices[2];
	output.Append(element);
}