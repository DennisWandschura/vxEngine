#pragma once

struct GpuLight;
struct Light;
class Frustum;
class RenderPassCullLights;
class RenderPassVisibleLights;
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
	RenderPassCullLights* m_renderPass;
	RenderPassVisibleLights* m_renderPassCopy;

public:
	LightManager();
	~LightManager();

	void getRequiredMemory(u64* heapSizeBuffere);

	bool initialize(vx::StackAllocator* allocator, u32 gpuLightCount, d3d::ResourceManager* resourceManager);

	bool loadSceneLights(const Light* lights, u32 count, ID3D12Device* device, d3d::ResourceManager* resourceManager, UploadManager* uploadManager);

	void __vectorcall update(__m128 cameraPosition, __m128 cameraDirection, const Frustum &frustum);

	void setRenderPasses(RenderPassCullLights* renderPass, RenderPassVisibleLights* renderPassCopy);

	void addStaticMeshInstance(const D3D12_DRAW_INDEXED_ARGUMENTS &cmd, const AABB &bounds, UploadManager* uploadManager);

	DrawIndexedIndirectCommand* getDrawCommand(u32 i);
};