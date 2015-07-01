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

#include "RenderAspect.h"
#include "d3dHelper.h"
#include <vxLib/Window.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/Scene.h>
#include "d3dx12.h"

#include <vxEngineLib/MeshFile.h>

struct Vertex
{
	vx::float3 position;
};

struct TaskLoadScene
{
	void* ptr;
};

const u32 g_swapChainBufferCount{ 2 };
const u32 g_maxVertexCount{ 10000 };
const u32 g_maxIndexCount{ 20000 };

void setResourceBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
	D3D12_RESOURCE_BARRIER descBarrier = {};

	descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	descBarrier.Transition.pResource = resource;
	descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	descBarrier.Transition.StateBefore = stateBefore;
	descBarrier.Transition.StateAfter = stateAfter;

	commandList->ResourceBarrier(1, &descBarrier);
}

RenderAspect::RenderAspect()
	:m_commandQueue(nullptr),
	m_commandList(nullptr),
	m_fence(nullptr),
	m_currentFence(0),
	m_handleEvent(nullptr),
	m_renderTarget(nullptr),
	m_device(nullptr),
	m_swapChain(nullptr),
	m_dxgiFactory(nullptr),
	m_lastSwapBuffer(0),
	m_descriptorHeapRtv(nullptr),
	m_commandAllocator(nullptr),
	m_heap(nullptr),
	m_vertexBuffer(nullptr),
	m_indexBuffer(nullptr)
{

}

RenderAspect::~RenderAspect()
{

}

bool RenderAspect::createCommandList()
{
	auto result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator, nullptr, IID_PPV_ARGS(&m_commandList));
	if (result != 0)
	{
		puts("error creating command list");
		return false;
	}

	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator, nullptr);

	m_commandList->SetGraphicsRootSignature(nullptr);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	setResourceBarrier(m_commandList, m_renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
	m_commandList->OMSetRenderTargets(1, &m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart(), true, nullptr);

	setResourceBarrier(m_commandList, m_renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_commandList->Close();

	return true;
}

bool RenderAspect::createMeshBuffers()
{
	/*auto result = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * g_maxVertexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,    // Clear value
		IID_PPV_ARGS(&m_vertexBuffer));

	if (result != 0)
	{
		puts("Error creating vertex buffer");
		return false;
	}*/

	D3D12_HEAP_DESC desc{};
	desc.Alignment = 0xffff + 1;
	desc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
	desc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	desc.SizeInBytes = sizeof(Vertex) * g_maxVertexCount + sizeof(u32) * g_maxIndexCount;
	auto result = m_device->CreateHeap(&desc, IID_PPV_ARGS(&m_heap));
	if (result != 0)
	{
		puts("Error creating buffer heap");
		printError(result);
		return false;
	}

	auto vresDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex));

	result = m_device->CreatePlacedResource(m_heap, 0, &vresDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_vertexBuffer));
	if (result != 0)
	{
		puts("Error creating vertex buffer");
		return false;
	}

	void* data = nullptr;
	m_vertexBuffer->Map(0, nullptr, &data);
	m_vertexBuffer->Unmap(0, nullptr);

	result = m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(u32) * g_maxIndexCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,    // Clear value
		IID_PPV_ARGS(&m_indexBuffer));

	if (result != 0)
	{
		puts("Error creating index buffer");
		return false;
	}

	return true;
}

bool RenderAspect::initialize(const RenderAspectDescription &desc)
{
	m_allocator = vx::StackAllocator(desc.pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);
	const auto doubleBufferSizeInBytes = 5 KBYTE;
	m_doubleBuffer = DoubleBufferRaw(&m_allocator, doubleBufferSizeInBytes);

	auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&m_device);
	if (result != 0)
	{
		puts("Error creating device");
		return false;
	}

	result = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));
	if (result != 0)
	{
		puts("Error creating dxgi factory");
		return false;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc;
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (result != 0)
	{
		puts("Error creating queue");
		return false;
	}

	m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator));

	m_viewport.Height = desc.settings->m_resolution.y;
	m_viewport.Width = desc.settings->m_resolution.x;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_rectScissor = { 0, 0, desc.settings->m_resolution.x, desc.settings->m_resolution.y };

	DXGI_SWAP_CHAIN_DESC descSwapChain;
	ZeroMemory(&descSwapChain, sizeof(descSwapChain));
	descSwapChain.BufferCount = g_swapChainBufferCount;
	descSwapChain.BufferDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	descSwapChain.OutputWindow = desc.window->getHwnd();
	descSwapChain.SampleDesc.Count = 1;
	descSwapChain.Windowed = 1;

	result = m_dxgiFactory->CreateSwapChain(
		m_commandQueue, // Swap chain needs the queue so it can force a flush on it
		&descSwapChain,
		&m_swapChain
		);
	if (result != 0)
	{
		puts("Error creating swap chain");
		printError(result);
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 1;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = m_device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&m_descriptorHeapRtv));
	if (result != 0)
	{
		puts("Error CreateDescriptorHeap");
		return false;
	}

	m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_renderTarget));
	m_device->CreateRenderTargetView(m_renderTarget, nullptr, m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart());

	if (!createCommandList())
		return false;

	if (!createMeshBuffers())
		return false;

	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_currentFence = 1;

	m_handleEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

	waitForGpu();

	return true;
}

void RenderAspect::waitForGpu()
{
	const UINT64 fence = m_currentFence;
	m_commandQueue->Signal(m_fence, fence);
	++m_currentFence;

	//
	// Let the previous frame finish before continuing
	//

	if (m_fence->GetCompletedValue() < fence)
	{
		m_fence->SetEventOnCompletion(fence, m_handleEvent);
		WaitForSingleObject(m_handleEvent, INFINITE);
	}
}

void RenderAspect::shutdown(void* hwnd)
{
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	if (m_heap)
	{
		m_heap->Release();

		m_heap = nullptr;
	}

	if (m_commandList)
	{
		m_commandList->Release();
		m_commandList = nullptr;
	}

	if (m_descriptorHeapRtv)
	{
		m_descriptorHeapRtv->Release();
		m_descriptorHeapRtv = nullptr;
	}

	if (m_fence)
	{
		m_fence->Release();
		m_fence = nullptr;
	}

	if (m_swapChain)
	{
		m_swapChain->Release();
		m_swapChain = nullptr;
	}

	if (m_commandQueue)
	{
		m_commandQueue->Release();
		m_commandQueue = nullptr;
	}

	if (m_dxgiFactory)
	{
		m_dxgiFactory->Release();
		m_dxgiFactory = nullptr;
	}

	if (m_commandAllocator)
	{
		m_commandAllocator->Release();
		m_commandAllocator = nullptr;
	}

	if (m_device)
	{
		m_device->Release();
		m_device = nullptr;
	}
}

bool RenderAspect::initializeProfiler()
{
	return true;
}

void RenderAspect::makeCurrent(bool b)
{
}

void RenderAspect::queueUpdateTask(const RenderUpdateTask &task)
{
	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_tasks.push_back(task);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize)
{
	vx::lock_guard<vx::mutex> lck(m_updateMutex);

	if (m_doubleBuffer.memcpy(data, dataSize))
	{
		m_tasks.push_back(task);
	}
	else
	{
		puts("RenderAspect::queueUpdateTask out of memory");
		std::exit(1);
	}
}

void RenderAspect::queueUpdateCamera(const RenderUpdateCameraData &data)
{
	RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::UpdateCamera;

	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_updateCameraData = data;

	m_tasks.push_back(task);
}

void RenderAspect::update()
{
	processTasks();
}

void RenderAspect::updateProfiler(f32 dt)
{

}

void RenderAspect::submitCommands()
{
	m_commandAllocator->Reset();

	{
		m_commandList->Reset(m_commandAllocator, nullptr);

		m_commandList->SetGraphicsRootSignature(nullptr);

		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_rectScissor);

		setResourceBarrier(m_commandList, m_renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_commandList->ClearRenderTargetView(m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);
		m_commandList->OMSetRenderTargets(1, &m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart(), true, nullptr);

		setResourceBarrier(m_commandList, m_renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_commandList->Close();
	}

	ID3D12CommandList* ppCommandLists[] = { m_commandList };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void RenderAspect::endFrame()
{
	m_swapChain->Present(1, 0);
	m_lastSwapBuffer = (1 + m_lastSwapBuffer) % g_swapChainBufferCount;
	m_swapChain->GetBuffer(m_lastSwapBuffer, IID_PPV_ARGS(&m_renderTarget));
	m_device->CreateRenderTargetView(m_renderTarget, nullptr, m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart());

	waitForGpu();
}

void RenderAspect::handleEvent(const vx::Event &evt)
{
	switch (evt.type)
	{
	case(vx::EventType::File_Event) :
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void RenderAspect::handleFileEvent(const vx::Event &evt)
{
	auto fileEvent = (vx::FileEvent)evt.code;

	switch (fileEvent)
	{
	case vx::FileEvent::Scene_Loaded:
	{
		auto pScene = (Scene*)evt.arg2.ptr;

		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::LoadScene;

		TaskLoadScene data;
		data.ptr = pScene;

		queueUpdateTask(task, (u8*)&data, sizeof(TaskLoadScene));
	}break;
	case vx::FileEvent::EditorScene_Loaded:
	{
		/*auto pScene = (Scene*)evt.arg2.ptr;

		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::LoadScene;

		TaskLoadScene data;
		data.ptr = pScene;
		data.editor = true;

		queueUpdateTask(task, (u8*)&data, sizeof(TaskLoadScene));*/
	}break;
	default:
		break;
	}
}

void RenderAspect::keyPressed(u16 key)
{

}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{

}

void RenderAspect::getTotalVRam(u32* totalVram) const
{

}

void RenderAspect::getTotalAvailableVRam(u32* totalAvailableVram) const
{

}

void RenderAspect::getAvailableVRam(u32* availableVram) const
{

}

void RenderAspect::processTasks()
{
	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_doubleBuffer.swapBuffers();

	auto backBuffer = m_doubleBuffer.getBackBuffer();
	auto backBufferSize = m_doubleBuffer.getBackBufferSize();
	u32 offset = 0;

	for (auto &it : m_tasks)
	{
		auto p = backBuffer + offset;

		switch (it.type)
		{
		case RenderUpdateTask::Type::UpdateCamera:
			taskUpdateCamera();
			break;
		case RenderUpdateTask::Type::UpdateDynamicTransforms:
			taskUpdateDynamicTransforms(p, &offset);
			break;
		case RenderUpdateTask::Type::UpdateText:
			taskUpdateText(p, &offset);
			break;
		case RenderUpdateTask::Type::CreateActorGpuIndex:
			taskCreateActorGpuIndex(p, &offset);
			break;
		case RenderUpdateTask::Type::TakeScreenshot:
			taskTakeScreenshot();
			break;
		case RenderUpdateTask::Type::LoadScene:
			taskLoadScene(p, &offset);
			break;
		case RenderUpdateTask::Type::ToggleRenderMode:
			taskToggleRenderMode();
			break;
		default:
			break;
		}
	}
	m_tasks.clear();

	VX_ASSERT(offset == backBufferSize);
}

void RenderAspect::taskUpdateCamera()
{
	m_camera.setPosition(m_updateCameraData.position);
	m_camera.setRotation(m_updateCameraData.quaternionRotation);

	/*auto projectionMatrix = m_renderContext.getProjectionMatrix();

	UniformCameraBufferBlock block;
	m_camera.getViewMatrix(&block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.position = m_camera.getPosition();
	block.qrotation = m_camera.getRotation();

	m_cameraBuffer.subData(0, sizeof(UniformCameraBufferBlock), &block);*/
}

void RenderAspect::taskUpdateText(u8* p, u32* offset)
{
	auto data = (RenderUpdateTextData*)p;

	//m_textRenderer->pushEntry(std::move(data->text), data->position, data->color);

	*offset += sizeof(RenderUpdateTextData);
}

void RenderAspect::taskTakeScreenshot()
{
}

void RenderAspect::taskLoadScene(u8* p, u32* offset)
{
	TaskLoadScene* data = (TaskLoadScene*)p;

	/*m_sceneRenderer.loadScene(data->ptr, m_objectManager, m_fileAspect, data->editor);

	auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);*/

	auto scene = (Scene*)data->ptr;

	auto &meshes = scene->getMeshes();
	auto meshCount = meshes.size();

	Vertex* gpuVertices = nullptr;
	auto error = m_vertexBuffer->Map(0, nullptr, (void**)&gpuVertices);
	VX_ASSERT(error == 0);

	u32 vertexOffset = 0;
	for (u32 i = 0; i < meshCount; ++i)
	{
		auto &meshFile = meshes[i];
		auto &mesh = meshFile->getMesh();

		auto vertexCount = mesh.getVertexCount();
		auto vertices = mesh.getVertices();

		for (u32 j = 0; j < vertexCount; ++j)
		{
			gpuVertices[vertexOffset].position = vertices[j].position;
			++vertexOffset;
		}
	}

	m_vertexBuffer->Unmap(0, nullptr);

	auto meshInstances = scene->getMeshInstances();
	auto meshInstanceCount = scene->getMeshInstanceCount();

	/*auto lightCount = scene->getLightCount();
	auto lights = scene->getLights();

	m_lightRenderer->setLights(lights, lightCount);

	if (m_shadowRenderer)
	{
		m_shadowRenderer->setLights(lights, lightCount);
	}*/

	*offset += sizeof(TaskLoadScene);
}

void RenderAspect::taskToggleRenderMode()
{
}

void RenderAspect::taskCreateActorGpuIndex(u8* p, u32* offset)
{
	/*std::size_t* address = (std::size_t*)p;

	CreateActorData* data = (CreateActorData*)(*address);
	auto gpuIndex = addActorToBuffer(data->getTransform(), data->getMeshSid(), data->getMaterialSid());
	data->setGpu(gpuIndex);

	vx::Event e;
	e.arg1.ptr = data;
	e.code = (u32)IngameEvent::Created_Actor_GPU;
	e.type = vx::EventType::Ingame_Event;

	m_evtManager->addEvent(e);*/

	*offset += sizeof(std::size_t);
}

void RenderAspect::taskUpdateDynamicTransforms(u8* p, u32* offset)
{
	auto data = (RenderUpdateDataTransforms*)p;
	u32 count = data->count;

	auto transforms = (vx::TransformGpu*)(data + 1);
	auto indices = (u32*)(p + sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count);

	/*for (u32 i = 0; i < count; ++i)
	{
		m_sceneRenderer.updateTransform(transforms[i], indices[i]);
	}*/

	*offset += sizeof(RenderUpdateDataTransforms) + (sizeof(vx::TransformGpu) + sizeof(u32)) * count;
}