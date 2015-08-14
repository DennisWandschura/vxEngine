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

struct ID3D12CommandQueue;
struct ID3D12Device;
struct IDXGISwapChain;
struct IDXGIFactory;
struct ID3D12Fence;
struct ID3D12DescriptorHeap;
struct ID3D12Resource;
struct ID3D12CommandAllocator;
struct ID3D12RootSignature;
struct ID3D12PipelineState;
struct ID3D12CommandSignature;
struct D3D12_SHADER_RESOURCE_VIEW_DESC;

class Scene;
class ResourceAspect;
struct ResourceView;

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
#include "DefaultRenderer.h"
#include "UploadManager.h"
#include "MeshManager.h"

class RenderAspect : public RenderAspectInterface
{
	d3d::Device m_device;
	std::mutex m_mutexCmdList;
	std::vector<DrawIndexedCommand> m_drawCommands;
	std::vector<ID3D12CommandList*> m_cmdLists;
	ID3D12GraphicsCommandList* m_commandList;
	d3d::Object<ID3D12Resource> m_renderTarget[2];
	d3d::Object<ID3D12Resource> m_indirectCmdBuffer;
	d3d::Object<ID3D12Resource> m_depthTexture;
	vx::sorted_vector<vx::StringID, ResourceView> m_resourceViews;
	u32 m_currentBuffer;
	u32 m_lastBuffer;
	d3d::Object<ID3D12CommandSignature> m_commandSignature;
	vx::Camera m_camera;
	DefaultRenderer m_defaultRenderer;
	vx::mat4 m_projectionMatrix;
	UploadManager m_uploadManager;
	d3d::Object<ID3D12Resource> m_constantBuffer;
	d3d::Object<ID3D12Resource> m_transformBuffer;
	d3d::DescriptorHeap m_descriptorHeapDsv;
	d3d::DescriptorHeap m_descriptorHeapRtv;
	d3d::DescriptorHeap m_descriptorHeapBuffer;
	d3d::Object<ID3D12CommandAllocator> m_commandAllocator;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_rectScissor;
	d3d::Heap m_bufferHeap;
	d3d::Object<ID3D12Resource> m_textureBuffer;
	d3d::Heap m_textureHeap;
	MeshManager m_meshManager;
	vx::TaskManager* m_taskManager;
	ResourceAspectInterface* m_resourceAspect;

	bool createHeaps();
	bool createCommandList();
	bool createMeshBuffers();
	bool createConstantBuffers();

	void handleFileMessage(const vx::Message &msg);

	void processTasks();
	void taskTakeScreenshot();
	void taskUpdateText(u8* p, u32* offset);
	void loadScene(Scene* scene);
	void taskToggleRenderMode();
	void taskCreateActorGpuIndex(u8* p, u32* offset);
	void taskUpdateDynamicTransforms(u8* p, u32* offset);

	void updateCamera(const RenderUpdateCameraData &data);

	void createCbvCamera();
	void updateSrvTransform(u32 instanceCount);

public:
	RenderAspect();
	~RenderAspect();

	RenderAspectInitializeError initialize(const RenderAspectDescription &desc);
	void shutdown(void* hwnd);

	bool initializeProfiler();

	void makeCurrent(bool b);

	void queueUpdateTask(const RenderUpdateTaskType type, const u8* data, u32 dataSize);
	void queueUpdateCamera(const RenderUpdateCameraData &data);
	void update();

	void updateProfiler(f32 dt);

	void submitCommands() override;
	void endFrame() override;

	void handleMessage(const vx::Message &msg) override;

	void keyPressed(u16 key);

	void getProjectionMatrix(vx::mat4* m);

	void getTotalVRam(u32* totalVram) const;
	void getTotalAvailableVRam(u32* totalAvailableVram) const;
	void getAvailableVRam(u32* availableVram) const;
};
