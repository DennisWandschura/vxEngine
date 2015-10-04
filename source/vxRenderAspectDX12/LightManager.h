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

struct GpuShadowTransformReverse;
struct GpuLight;
class Frustum;
struct GpuShadowTransform;
struct D3D12_DRAW_INDEXED_ARGUMENTS;
struct AABB;
struct RenderSettings;
class RenderPassLight;
class RenderAspect;

namespace Graphics
{
	struct Light;
	struct LightGeometryProxy;
}

namespace d3d
{
	class ResourceManager;
}

#include <vxLib/math/Vector.h>
#include <vxLib/Allocator/StackAllocator.h>
#include "DrawIndexedIndirectCommand.h"
#include <vxEngineLib/Event.h>
#include <memory>

class LightManager
{	
	GpuLight* m_sceneLights;
	GpuShadowTransform* m_sceneShadowTransforms;
	GpuShadowTransformReverse* m_sceneShadowReverseTransforms;
	__m128* m_sceneLightBounds;
	std::unique_ptr<Graphics::LightGeometryProxy[]> m_proxies;
	GpuLight* m_gpuLights;
	GpuShadowTransform* m_gpuShadowTransforms;
	GpuShadowTransformReverse* m_gpuShadowReverseTransforms;
	u32 m_proxyCount;
	u32 m_visibleLightCount;
	u32 m_sceneLightCount;
	u32 m_maxSceneLightCount;
	u32 m_maxShadowCastingLights;
	u32 m_resultBufferCount;
	u32* m_visibleLightsResult;
	Event m_downloadEvent;
	Event m_checkLightsEvent;
	std::vector<RenderPassLight*> m_renderPasses;
	vx::StackAllocator m_scratchAllocator;

	void createSrvLights(u32 maxCount, d3d::ResourceManager* resourceManager);
	void createSrvShadowCastingLights(u32 maxCount, d3d::ResourceManager* resourceManager);

public:
	LightManager();
	LightManager(const LightManager&) = delete;
	LightManager(LightManager &&rhs);
	~LightManager();

	void getRequiredMemory(u64* heapSizeBuffere, u32* bufferCount, u32 maxSceneLightCount, u32 maxShadowCastingLights);

	bool initialize(const RenderSettings &settings, vx::StackAllocator* allocator, u32 maxSceneLightCount, d3d::ResourceManager* resourceManager, UploadManager* uploadManager);

	bool loadSceneLights(const Graphics::Light* lights, u32 lightCount, Graphics::LightGeometryProxy* proxies, u32 proxyCount, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager);

	void __vectorcall update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum, d3d::ResourceManager* resourceManager, UploadManager* uploadManager, RenderAspect* renderAspect);

	void addStaticMeshInstance(const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, const AABB &bounds, UploadManager* uploadManager);

	void addRenderPass(RenderPassLight* rp) { m_renderPasses.push_back(rp); }
};