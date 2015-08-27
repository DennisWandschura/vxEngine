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

struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12CommandSignature;

class Scene;
class ResourceAspect;
struct ResourceView;
struct Light;
class RenderPass;

namespace vx
{
	class MessageManager;
}

#include <vxEngineLib/RenderAspectInterface.h>
#include <d3d12.h>
#include <vxEngineLib/mutex.h>
#include <vxEngineLib/DoubleBufferRaw.h>
#include <vxLib/Graphics/Camera.h>
#include <vector>
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "Device.h"
#include "Heap.h"
#include "DescriptorHeap.h"
#include <mutex>
#include "UploadManager.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "ShaderManager.h"
#include "ResourceManager.h"
#include "Debug.h"

class VX_ALIGN(32) RenderAspect : public RenderAspectInterface
{
	d3d::Device m_device;
	std::mutex m_mutexCmdList;
	std::vector<DrawIndexedCommand> m_drawCommands;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12GraphicsCommandList* m_commandCopyBuffers;
	ID3D12Resource* m_renderTarget[2];
	d3d::ResourceManager m_resourceManager;
	u32 m_currentBuffer;
	u32 m_lastBuffer;
	d3d::Object<ID3D12CommandSignature> m_commandSignature;
	vx::Camera m_camera;
	std::vector<RenderPass*> m_renderPasses;
	d3d::Debug m_debug;
	vx::mat4d m_projectionMatrix;
	vx::mat4d m_viewMatrixPrev;
	f32 m_zFar;
	f32 m_zNear;
	vx::uint2 m_resolution;
	UploadManager m_uploadManager;
	d3d::DescriptorHeap m_descriptorHeapRtv;
	d3d::Object<ID3D12CommandAllocator> m_commandAllocator;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_rectScissor;
	MeshManager m_meshManager;
	MaterialManager m_materialManager;
	vx::TaskManager* m_taskManager;
	ResourceAspectInterface* m_resourceAspect;
	vx::StackAllocator m_allocator;
	vx::MessageManager* m_msgManager;
	d3d::ShaderManager m_shaderManager;
	std::vector<u32> m_copyTransforms;

	void createRenderPasses(const vx::uint2 &resolution);
	void getRequiredMemory(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, u64* bufferHeapSize, u64* textureHeapSize, u64* rtDsHeapSize);
	bool initializeRenderPasses();

	bool createCommandList();
	bool createConstantBuffers();

	void handleFileMessage(const vx::Message &msg);

	void processTasks();
	void taskTakeScreenshot();
	void taskUpdateText(const u8* p, u32* offset);
	void loadScene(Scene* scene);
	void taskToggleRenderMode();
	void taskCreateActorGpuIndex(const u8* p, u32* offset);
	void taskUpdateDynamicTransforms(const u8* p, u32* offset);
	void taskAddStaticMeshInstance(const u8* p, u32* offset);
	void taskAddDynamicMeshInstance(const u8* p, u32* offset);

	void updateCamera(const RenderUpdateCameraData &data);

	void createCbvCamera();
	void createSrvTransformPrev(u32 instanceCount);
	void createSrvTransform(u32 instanceCount);
	void createSrvMaterial(u32 instanceCount);
	void createSrvTextures(u32 srgbCount, u32 rgbCount);

	void addMeshInstance(const MeshInstance &meshInstance, u32* gpuIndex);

	void drawScreenQuadGBuffer(ID3D12GraphicsCommandList* cmdList);

	void updateTransform(const vx::Transform &transform, u32 index);
	void updateTransformStatic(const vx::TransformGpu &transform, u32 index);
	void updateTransformDynamic(const vx::TransformGpu &transform, u32 index);

	void copyTransform(u32 index);

	void renderGBuffer();
	void copyTransforms(ID3D12GraphicsCommandList* cmdList);
	void updateLights(const Light* lights, u32 count);

public:
	RenderAspect();
	~RenderAspect();

	RenderAspectInitializeError initialize(const RenderAspectDescription &desc);
	void shutdown(void* hwnd);

	bool initializeProfiler(Logfile* errorlog);

	void makeCurrent(bool b);

	void queueUpdateTask(const RenderUpdateTaskType type, const u8* data, u32 dataSize);
	void queueUpdateCamera(const RenderUpdateCameraData &data);
	void update();

	void updateProfiler(f32 dt);

	void submitCommands() override;
	void endFrame() override;

	void handleMessage(const vx::Message &msg) override;

	void keyPressed(u16 key);

	void getProjectionMatrix(vx::mat4* m) override;

	void getTotalVRam(u32* totalVram) const;
	void getTotalAvailableVRam(u32* totalAvailableVram) const;
	void getAvailableVRam(u32* availableVram) const;
};
