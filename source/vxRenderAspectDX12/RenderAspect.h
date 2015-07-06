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

#include <vxEngineLib/RenderAspectInterface.h>
#include <d3d12.h>
#include <vxEngineLib/mutex.h>
#include <vxEngineLib/DoubleBufferRaw.h>
#include <vxLib/Graphics/Camera.h>
#include <vector>
#include "UploadHeap.h"
#include <vxLib/Container/sorted_vector.h>
#include <vxLib/StringID.h>
#include "DefaultHeap.h"

struct MeshEntry
{
	u32 indexStart;
	u32 indexCount;
};

struct MeshInstanceDrawCmd
{
	u32 indexCount;
	u32 instanceCount;
	u32 firstIndex;
	u32 baseVertex;
	u32 baseInstance;
};

class RenderAspect : public RenderAspectInterface
{
	ID3D12CommandQueue* m_commandQueue;
	ID3D12GraphicsCommandList* m_commandList;
	ID3D12Fence* m_fence;
	u64 m_currentFence;
	void* m_handleEvent;
	ID3D12Resource* m_renderTarget;
	vx::Camera m_camera;
	ID3D12Device* m_device;
	IDXGISwapChain* m_swapChain;
	IDXGIFactory1* m_dxgiFactory;
	u32 m_lastSwapBuffer;
	ID3D12DescriptorHeap* m_descriptorHeapRtv;
	ID3D12CommandAllocator* m_commandAllocator;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_rectScissor;
	UploadHeap m_uploadHeap;
	UploadBuffer m_vertexUploadBuffer;
	UploadBuffer m_indexUploadBuffer;
	vx::mutex m_updateMutex;
	std::vector<RenderUpdateTask> m_tasks;
	DoubleBufferRaw m_doubleBuffer;
	RenderUpdateCameraData m_updateCameraData;
	vx::StackAllocator m_allocator;
	u32 m_meshIndexOffset;
	vx::sorted_vector<vx::StringID, MeshEntry> m_meshEntries;
	std::vector<MeshInstanceDrawCmd> m_drawCommands;
	DefaultHeap m_defaultBufferHeap;
	DefaultHeap m_defaultGeometryHeap;

	bool createHeaps();
	bool createCommandList();
	bool createMeshBuffers();

	void waitForGpu();

	void handleFileEvent(const vx::Event &evt);

	void processTasks();
	void taskUpdateCamera();
	void taskTakeScreenshot();
	void taskUpdateText(u8* p, u32* offset);
	void taskLoadScene(u8* p, u32* offset);
	void taskToggleRenderMode();
	void taskCreateActorGpuIndex(u8* p, u32* offset);
	void taskUpdateDynamicTransforms(u8* p, u32* offset);

public:
	RenderAspect();
	~RenderAspect();

	RenderAspectInitializeError initialize(const RenderAspectDescription &desc);
	void shutdown(void* hwnd);

	bool initializeProfiler();

	void makeCurrent(bool b);

	void queueUpdateTask(const RenderUpdateTask &task);
	void queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize);
	void queueUpdateCamera(const RenderUpdateCameraData &data);
	void update();

	void updateProfiler(f32 dt);

	void submitCommands();
	void endFrame();

	void handleEvent(const vx::Event &evt) override;

	void keyPressed(u16 key);

	void getProjectionMatrix(vx::mat4* m);

	void getTotalVRam(u32* totalVram) const;
	void getTotalAvailableVRam(u32* totalAvailableVram) const;
	void getAvailableVRam(u32* availableVram) const;
};
