#pragma once

#include <vxLib/math/matrix.h>

struct Camerablock
{
	vx::mat4 pvMatrix;
	vx::mat4 viewMatrix;
	vx::mat4 inversePVMatrix;
	__m128 cameraPosition;
	__m128 padding[3];
};

struct CamerablockStatic
{
	vx::mat4 invProjectionMatrix;
	vx::mat4 projectionMatrix;
	vx::mat4 orthoMatrix;
};

struct VX_ALIGN(32) VoxelBlock
{
	vx::mat4 projectionMatrix;
	U32 dim;
	U32 halfDim;
	float gridCellSize;
	float invGridCellSize;
};

struct LightData
{
	vx::float3 position; 
	float falloff; 
	vx::float3 direction;
	float lumen;
};

struct PointLightShadowTransform
{
	vx::mat4 projectionMatrix;
	vx::mat4 viewMatrix[6];
	vx::mat4 pvMatrices[6];
};

struct ShadowTransformBlock
{
	PointLightShadowTransform transforms[5];
};

struct LightDataBlock
{
	LightData u_lightData[5];
	U32 size;
};

struct MaterialGPU
{
	U32 indexAlbedo;
	U32 indexNormal;
	U32 indexSurface;
	U32 hasNormalMap;
};