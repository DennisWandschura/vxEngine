#pragma once
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


#include <UniformCameraBuffer.h>
#include <UniformCameraBufferStatic.h>
#include <UniformShadowTransformBuffer.h>

namespace Gpu
{
	struct VoxelData
	{
		vx::mat4 projectionMatrix;
		u32 dim;
		u32 halfDim;
		float gridCellSize;
		float invGridCellSize;
	};

	struct VoxelBlock
	{
		VoxelData data[4];
	};

	struct LightData
	{
		__m128 position;
		float falloff;
		float lumen;
		f32 padding[2];
	};

	struct UniformGBufferBlock
	{
		u64 u_albedoSlice;
		u64 u_normalSlice;
		u64 u_surfaceSlice;
		u64 u_tangentSlice;
		u64 u_bitangentSlice;
		u64 u_depthSlice;
	};

	struct UniformTextureBufferBlock
	{
		u64 u_aabbTexture;
		u64 u_ambientSlice;
		u64 u_ambientImage;
		u64 u_volumetricTexture;
		u64 u_particleTexture;
	};

	struct MaterialGPU
	{
		u32 indexAlbedo;
		u32 indexNormal;
		u32 indexSurface;
		u32 hasNormalMap;
	};

	struct RenderSettingsBlock
	{
		vx::uint2 resolution;
	};
}