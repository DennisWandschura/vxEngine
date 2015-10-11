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

class Scene;
class UploadManager;
class MaterialManager;
class CreateActorData;
class CreateDynamicMeshData;
class CopyManager;
class DownloadManager;
class CpuProfiler;

namespace d3d
{
	class ResourceManager;
	class Device;
}

namespace vx
{
	class Camera;
	class MessageManager;
}

#include <vxEngineLib/Graphics/RenderLayer.h>
#include "RenderPass.h"
#include <vector>
#include <memory>
#include "LightManager.h"
#include "MeshManager.h"
#include "CommandList.h"
#include "CommandAllocator.h"
#include "DrawIndexedIndirectCommand.h"
#include "Frustum.h"
#include "RenderStage.h"

struct RenderLayerGameDesc
{
	d3d::Device* m_device;
	CopyManager* m_copyManager;
	UploadManager* m_uploadManager;
	vx::MessageManager* m_msgManager;
	d3d::ResourceManager* m_resourceManager;
	vx::Camera* m_camera;
	Frustum* m_frustum;
	MaterialManager* m_materialManager;
	ResourceAspectInterface* m_resourceAspect;
	DownloadManager* m_downloadManager;
	RenderSettings* m_settings;
	CpuProfiler* m_cpuProfiler;
	RenderAspect* m_renderAspect;
};

class RenderLayerGame : public Graphics::RenderLayer
{
	std::vector<RenderStage> m_renderStages;
	CopyManager* m_copyManager;
	std::unique_ptr<d3d::CommandAllocator[]> m_allocators;
	LightManager m_lightManager;
	vx::Camera* m_camera;
	Frustum* m_frustum;
	f32 m_gridCellSize;
	f32 m_invGridCellSize;
	vx::int3 m_lastVoxelCenter;
	RenderAspect* m_renderAspect;
	CpuProfiler* m_cpuProfiler;
	DrawIndexedIndirectCommand m_drawCommandMesh;
	d3d::Device* m_device;
	UploadManager* m_uploadManager;
	vx::MessageManager* m_msgManager;
	d3d::ResourceManager* m_resourceManager;
	MaterialManager* m_materialManager;
	ResourceAspectInterface* m_resourceAspect;
	MeshManager m_meshManager;
	DownloadManager* m_downloadManager;
	RenderSettings* m_settings;
	vx::sorted_vector<vx::StringID, std::unique_ptr<RenderPass>> m_renderPasses;

	void createGpuObjects();

	void handleRendererMessage(const vx::Message &msg);
	void handleFileMessage(const vx::Message &msg);
	void loadScene(Scene* scene);

	D3D12_DRAW_INDEXED_ARGUMENTS addMeshInstance(const MeshInstance &meshInstance, u32* gpuIndex);
	void addStaticMeshInstance(const MeshInstance &instance, const vx::StringID &sid);

	void createActorGpuIndex(CreateActorData* data);
	void addDynamicMeshInstance(CreateDynamicMeshData* data);

	void updateTransform(const vx::Transform &transform, u32 index);
	void updateTransformStatic(const vx::TransformGpu &transform, u32 index);
	void updateTransformDynamic(const vx::TransformGpu &transform, u32 index);
	void taskUpdateDynamicTransforms(const u8* p, u32* offset);

	void copyTransform(u32 index);

	void insertRenderPass(vx::StringID &&sid, std::unique_ptr<RenderPass> &&renderPass);
	void insertRenderPass(const char* id, std::unique_ptr<RenderPass> &&renderPass);

	RenderPass* findRenderPass(const char* id);

public:
	explicit RenderLayerGame(const RenderLayerGameDesc &desc);
	RenderLayerGame(const RenderLayerGame&) = delete;
	RenderLayerGame(RenderLayerGame &&rhs);
	~RenderLayerGame();

	RenderLayerGame& operator=(const RenderLayerGame&) = delete;

	void createRenderPasses() override;

	void getRequiredMemory(u64* heapSizeBuffer, u32* bufferCount, u64* heapSizeTexture, u32* textureCount, u64* heapSizeRtDs, u32* rtDsCount) override;
	bool createData() override;

	bool initialize(u32 frameCount, vx::StackAllocator* allocator, Logfile* errorLog) override;
	void shudown() override;

	void update() override;

	void queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize) override;

	void buildCommandLists(u32 frameIndex) override;
	void submitCommandLists(Graphics::CommandQueue* queue) override;
	void buildAndSubmitCommandLists(u32 frameIndex, Graphics::CommandQueue* queue) override;

	u32 getCommandListCount() const override;

	void handleMessage(const vx::Message &evt) override;
};