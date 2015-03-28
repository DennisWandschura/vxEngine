#pragma once

#include <vxLib/math/matrix.h>
#include "Transform.h"

struct LightTile
{
	__m128 size;
	__m128 positionFalloff[3];
};

struct VX_ALIGN(16) BVHNodeGpu
{
	vx::float3 bmin;
	//U32 primOffset_secondChildOffset;
	union
	{
		U32 triangleOffset; // leaf
		U32 secondChildOffset; // interior
	};
	vx::float3 bmax;

	//U32 primCount_axis_leaf;
	U16 triangleCount; // if 0, interior
	U8 axis;
	U8 pad;
};

struct Camerablock
{
	vx::mat4 pvMatrix;
	vx::mat4 viewMatrix;
	vx::mat4 inversePVMatrix;
	__m128 cameraPosition;
	__m128 padding[3];
};

struct CamerablockCS
{
	vx::mat4 inversePVMatrix;
	__m128 cameraPosition;
};

struct CamerablockStatic
{
	vx::mat4 invProjectionMatrix;
	vx::mat4 projectionMatrix;
	vx::mat4 orthoMatrix;
};

struct VoxelBlock
{
	vx::mat4 projectionMatrices[3];
	U32 dim;
	U32 halfDim;
	float gridCellSize;
	float invGridCellSize;
};

struct VoxelBlockReflect
{
	vx::mat4 projectionMatrices[3];
	U64 samplerVoxel;
	U64 samplerOpacity;
	U32 dim;
	U32 halfDim;
	float gridCellSize;
	float invGridCellSize;
};

struct ShadowTransformBlock
{
	vx::mat4 pvMatrix[60];
	vx::float4 position[60];
};

struct SphereLightData
{
	vx::float4 position;
	float falloff;
	float surfaceRadius;
	float lumen;
	float _padding;
};

struct LightDataBlock
{
	SphereLightData u_lightData[10];
	U32 size;
};

struct MaterialGPU
{
	U32 indexAlbedo;
	U32 indexNormal;
	U32 indexSurface;
	U32 hasNormalMap;
};

struct AABB_GPU
{
	vx::float4 vmin;
	vx::float4 vmax;
};

struct Tile
{
	U32 size;
	U32 lights[11];
};

struct VX_ALIGN(16) CompressedRay
{
	/*vx::float3 origin;
	U32 mortonCode;
	vx::float3 direction;
	F32 distance;*/
	vx::float3 origin;
	int texCoords;
	vx::float3 direction;
	float distance;
	//U32 compressedRay[3];
};

struct RayLink
{
	U32 rayIndex;
};