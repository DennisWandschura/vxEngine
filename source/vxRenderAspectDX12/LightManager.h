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

struct GpuLight;
struct Light;
class Frustum;
struct ShadowTransform;
struct D3D12_DRAW_INDEXED_ARGUMENTS;
struct AABB;

namespace d3d
{
	class ResourceManager;
}

#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>
#include "DrawIndexedIndirectCommand.h"

class LightManager
{
	GpuLight* m_sceneLights;
	ShadowTransform* m_sceneShadowTransforms;
	__m128* m_sceneLightBounds;
	GpuLight* m_gpuLights;
	u32 m_sceneLightCount;
	u32 m_gpuLightCount;
	DrawIndexedIndirectCommand m_drawCommand;
	vx::StackAllocator m_scratchAllocator;

	void createSrvLights(u32 maxCount, d3d::ResourceManager* resourceManager);

public:
	LightManager();
	LightManager(const LightManager&) = delete;
	LightManager(LightManager &&rhs);
	~LightManager();

	void getRequiredMemory(u64* heapSizeBuffere, u32 maxLightCountGpu);

	bool initialize(vx::StackAllocator* allocator, u32 gpuLightCount, d3d::ResourceManager* resourceManager);

	bool loadSceneLights(const Light* lights, u32 count, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager);

	void __vectorcall update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum);

	void addStaticMeshInstance(const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, const AABB &bounds, UploadManager* uploadManager);

	DrawIndexedIndirectCommand* getDrawCommand(u32 i);
};