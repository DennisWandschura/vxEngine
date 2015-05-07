/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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
	u32 dim;
	u32 halfDim;
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

struct UniformTextureBufferBlock
{
	u64 u_albedoSlice;
	u64 u_normalSlice;
	u64 u_surfaceSlice;
	u64 u_tangentSlice;
	u64 u_depthSlice;
	u64 u_aabbTexture;
	u64 u_ambientSlice;
};

struct UniformShadowTextureBufferBlock
{
	u64 u_shadowTextures[5];
};

struct LightDataBlock
{
	LightData u_lightData[5];
	u32 size;
};

struct MaterialGPU
{
	u32 indexAlbedo;
	u32 indexNormal;
	u32 indexSurface;
	u32 hasNormalMap;
};