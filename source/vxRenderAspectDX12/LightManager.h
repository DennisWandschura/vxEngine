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

class RenderPassShadow;
class RenderPassShading;
class RenderPassCullLights;
struct GpuShadowTransformReverse;
struct GpuLight;
struct Light;
class Frustum;
struct GpuShadowTransform;
struct D3D12_DRAW_INDEXED_ARGUMENTS;
struct AABB;
class RenderPassFilterRSM;

namespace d3d
{
	class ResourceManager;
}

#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>
#include "DrawIndexedIndirectCommand.h"
#include <vxEngineLib/Event.h>

class LightManager
{
	GpuLight* m_sceneLights;
	GpuShadowTransform* m_sceneShadowTransforms;
	GpuShadowTransformReverse* m_sceneShadowReverseTransforms;
	__m128* m_sceneLightBounds;
	GpuLight* m_gpuLights;
	GpuShadowTransform* m_gpuShadowTransforms;
	GpuShadowTransformReverse* m_gpuShadowReverseTransforms;
	u32 m_sceneLightCount;
	u32 m_maxSceneLightCount;
	u32 m_maxShadowCastingLights;
	u32 m_resultBufferCount;
	u32* m_visibleLightsResult;
	Event m_downloadEvent;
	Event m_checkLightsEvent;
	RenderPassShadow* m_renderPassShadow;
	RenderPassShading* m_renderPassShading;
	RenderPassCullLights* m_renderPassCullLights;
	RenderPassFilterRSM* m_renderPassFilterRSM;
	vx::StackAllocator m_scratchAllocator;

	void createSrvLights(u32 maxCount, d3d::ResourceManager* resourceManager);
	void createSrvShadowCastingLights(u32 maxCount, d3d::ResourceManager* resourceManager);

public:
	LightManager();
	LightManager(const LightManager&) = delete;
	LightManager(LightManager &&rhs);
	~LightManager();

	void getRequiredMemory(u64* heapSizeBuffere, u32 maxSceneLightCount, u32 maxShadowCastingLights);

	bool initialize(vx::StackAllocator* allocator, u32 maxSceneLightCount, u32 maxShadowCastingLights, d3d::ResourceManager* resourceManager);

	bool loadSceneLights(const Light* lights, u32 count, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager);

	void __vectorcall update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum, d3d::ResourceManager* resourceManager, UploadManager* uploadManager);

	void addStaticMeshInstance(const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, const AABB &bounds, UploadManager* uploadManager);

	void setRenderPassShadow(RenderPassShadow* renderPassShadow) { m_renderPassShadow = renderPassShadow; }
	void setRenderPassShading(RenderPassShading* renderPassShading) { m_renderPassShading = renderPassShading; }
	void setRenderPassCullLights(RenderPassCullLights* renderPassCullLights);
	void setRenderPassFilterRSM(RenderPassFilterRSM* renderPassFilterRSM) { m_renderPassFilterRSM = renderPassFilterRSM; }
};