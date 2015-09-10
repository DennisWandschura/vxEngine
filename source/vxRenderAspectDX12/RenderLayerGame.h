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
};

class RenderLayerGame : public Graphics::RenderLayer
{
	std::vector<std::unique_ptr<RenderPass>> m_renderPasses;
	CopyManager* m_copyManager;
	d3d::CommandAllocator m_commandAllocator;
	LightManager m_lightManager;
	vx::Camera* m_camera;
	Frustum* m_frustum;
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

public:
	explicit RenderLayerGame(const RenderLayerGameDesc &desc);
	RenderLayerGame(const RenderLayerGame&) = delete;
	RenderLayerGame(RenderLayerGame &&rhs);
	~RenderLayerGame();

	RenderLayerGame& operator=(const RenderLayerGame&) = delete;

	void createRenderPasses() override;

	void getRequiredMemory(u64* heapSizeBuffer, u64* heapSizeTexture, u64* heapSizeRtDs) override;

	bool initialize(vx::StackAllocator* allocator) override;
	void shudown() override;

	void update() override;

	void queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize) override;

	void submitCommandLists(Graphics::CommandQueue* queue) override;

	u32 getCommandListCount() const override;

	void pushRenderPass(std::unique_ptr<RenderPass> &&renderPass);

	void handleMessage(const vx::Message &evt) override;
};