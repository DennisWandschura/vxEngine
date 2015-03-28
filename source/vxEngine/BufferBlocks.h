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

struct SphereLightShadowTransformBlock
{
	vx::mat4 pvMatrix[60];
	vx::float4 position[60];
};

struct SpotLightData
{
	vx::mat4 projectionMatrix;
	vx::float4 position;
	vx::float3 direction;
	F32 angle;
};

struct SphereLightData
{
	vx::mat4 projectionMatrix;
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