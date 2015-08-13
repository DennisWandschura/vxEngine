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
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/Scene.h>
#include "d3dx12.h"
#include "TaskUpdateCamera.h"
#include <vxEngineLib/TaskManager.h>
#include "TaskUploadGeometry.h"
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>
#include <dxgi1_4.h>

#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/MeshFile.h>

// Graphics root signature parameter offsets.
enum GraphicsRootParameters
{
	Cbv,
	GraphicsRootParametersCount
};

struct IndirectCommand
{
	D3D12_GPU_VIRTUAL_ADDRESS cbv;
	D3D12_DRAW_ARGUMENTS drawArguments;
};

struct Vertex
{
	vx::float3 position;
	vx::float3 color;
};

struct CameraBufferData
{
	vx::float4a cameraPosition;
	vx::mat4 pvMatrix;
	vx::mat4 cameraViewMatrix;
};

struct Transform
{
	vx::float4 translation;
};

struct TaskLoadScene
{
	void* ptr;
};

const u32 g_swapChainBufferCount{ 2 };
const u32 g_maxVertexCount{ 20000 };
const u32 g_maxIndexCount{ 40000 };
const u32 g_maxMeshInstances{ 128 };

namespace RenderAspectCpp
{
	template<u32 SIZE>
	void submitCommandLists(ID3D12CommandList*(&lists)[SIZE], d3d::Device* device)
	{
		device->executeCommandLists(SIZE, lists);
	}
}

RenderAspect::RenderAspect()
	:m_device(),
	m_commandList(nullptr),
	m_indexCount(0),
	m_vertexCount(0),
	m_instanceCount(0),
	m_renderTarget(),
	m_descriptorHeapRtv(),
	m_commandAllocator(),
	m_taskManager(nullptr)
{

}

RenderAspect::~RenderAspect()
{

}

bool RenderAspect::createCommandList()
{
	auto device = m_device.getDevice();
	auto result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(&m_commandList));
	if (result != 0)
	{
		puts("error creating command list");
		return false;
	}

	m_commandList->Close();

	return true;
}

bool RenderAspect::createHeaps()
{
	if (!m_bufferHeap.createBufferHeap(64u KBYTE * 4, D3D12_HEAP_TYPE_DEFAULT, &m_device))
		return false;

	auto geometryHeapSize = d3d::getAlignedSize(sizeof(Vertex) * g_maxVertexCount, 64 KBYTE) + d3d::getAlignedSize(sizeof(u32) * g_maxIndexCount, 64 KBYTE);
	if (!m_geometryHeap.createBufferHeap(geometryHeapSize, D3D12_HEAP_TYPE_UPLOAD, &m_device))
		return false;

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 16;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeap.NodeMask = 1;
	if (!m_descriptorHeapBuffer.create(descHeap, &m_device))
		return false;

	return true;
}

bool RenderAspect::createMeshBuffers()
{
	return true;
}

bool RenderAspect::createConstantBuffers()
{
	if (!m_bufferHeap.createResourceBuffer(64u KBYTE, 0, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_constantBuffer.getAddressOf(), &m_device))
		return false;

	if (!m_bufferHeap.createResourceBuffer(64u KBYTE, 64u KBYTE, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, m_indirectCmdBuffer.getAddressOf(), &m_device))
		return false;

	auto vertexBufferSize = d3d::getAlignedSize(sizeof(Vertex) * g_maxVertexCount, 64 KBYTE);
	if (!m_geometryHeap.createResourceBuffer(vertexBufferSize, 0, D3D12_RESOURCE_STATE_GENERIC_READ, m_vertexBuffer.getAddressOf(), &m_device))
		return false;

	auto indexBufferSize = d3d::getAlignedSize(sizeof(u32) * g_maxIndexCount, 64 KBYTE);
	if (!m_geometryHeap.createResourceBuffer(indexBufferSize, vertexBufferSize, D3D12_RESOURCE_STATE_GENERIC_READ, m_indexBuffer.getAddressOf(), &m_device))
		return false;

	return true;
}

RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &desc)
{
	m_taskManager = desc.taskManager;

	auto screenAspect = (f32)desc.settings->m_resolution.x / (f32)desc.settings->m_resolution.y;
	auto fovRad = vx::degToRad(desc.settings->m_fovDeg);
	m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(fovRad, screenAspect, desc.settings->m_zNear, desc.settings->m_zFar);

	const auto doubleBufferSizeInBytes = 5 KBYTE;

	if (!m_device.initialize(*desc.window))
	{
		printf("error initializing device\n");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	auto device = m_device.getDevice();

	auto hresult = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.getAddressOf()));
	if (hresult != 0)
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	m_viewport.Height = (f32)desc.settings->m_resolution.y;
	m_viewport.Width = (f32)desc.settings->m_resolution.x;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_rectScissor.left = 0;
	m_rectScissor.top = 0;
	m_rectScissor.right = desc.settings->m_resolution.x;
	m_rectScissor.bottom = desc.settings->m_resolution.y;

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 2;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (!m_descriptorHeapRtv.create(descHeap, &m_device))
	{
		puts("Error CreateDescriptorHeap");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	auto rtvHandleCpu = m_descriptorHeapRtv.getHandleCpu();
	auto swapChain = m_device.getSwapChain();
	m_currentBuffer = 0;
	m_lastBuffer = 1;
	for (u32 i = 0; i < 2; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(m_renderTarget[i].getAddressOf()));

		device->CreateRenderTargetView(m_renderTarget[i].get(), nullptr, rtvHandleCpu);
		rtvHandleCpu.offset(1);
	}

	if (!createHeaps())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createCommandList())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createMeshBuffers())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createConstantBuffers())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!m_defaultRenderer.initialize(device))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_uploadManager.initialize(&m_device))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
	//argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_CONSTANT_BUFFER_VIEW;
	//argumentDescs[0].ConstantBufferView.RootParameterIndex = Cbv;
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc;
	cmdSigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
	cmdSigDesc.NodeMask = 0;
	cmdSigDesc.NumArgumentDescs = 1;
	cmdSigDesc.pArgumentDescs = argumentDescs;
	hresult = device->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(m_commandSignature.getAddressOf()));

	{
		D3D12_DRAW_INDEXED_ARGUMENTS cmd{};
		m_uploadManager.pushUpload((u8*)&cmd, m_indirectCmdBuffer.get(), 0, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

		u32 count = 0;
		m_uploadManager.pushUpload((u8*)&count, m_indirectCmdBuffer.get(), 256, sizeof(u32), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

		auto list = m_uploadManager.update();

		ID3D12CommandList* lists[]=
		{
			list
		};

		m_device.executeCommandLists(1, lists);
		m_device.waitForGpu();
	}

	return RenderAspectInitializeError::OK;
}

void RenderAspect::shutdown(void* hwnd)
{
	if (m_commandList)
	{
		m_commandList->Release();
		m_commandList = nullptr;
	}

	m_descriptorHeapRtv.destroy();

	m_commandAllocator.destroy();

	m_device.shutdown();
}

bool RenderAspect::initializeProfiler()
{
	return true;
}

void RenderAspect::makeCurrent(bool b)
{
}

void RenderAspect::updateCamera(const RenderUpdateCameraData &data)
{
	m_camera.setPosition(data.position);
	m_camera.setRotation(data.quaternionRotation);

	vx::mat4 viewMatrix;
	m_camera.getViewMatrix(&viewMatrix);

	CameraBufferData bufferData;
	bufferData.cameraPosition = m_camera.getPosition();
	bufferData.cameraViewMatrix = viewMatrix;
	bufferData.pvMatrix = m_projectionMatrix * viewMatrix;

	u32 offset = m_currentBuffer * d3d::getAlignedSize(sizeof(CameraBufferData), 256);
	m_uploadManager.pushUpload((u8*)&bufferData, m_constantBuffer.get(), offset, sizeof(CameraBufferData), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTaskType type, const u8* data, u32 dataSize)
{
	switch (type)
	{
	case RenderUpdateTaskType::UpdateCamera:
		break;
	case RenderUpdateTaskType::UpdateDynamicTransforms:
		break;
	case RenderUpdateTaskType::UpdateText:
		break;
	case RenderUpdateTaskType::LoadScene:
		break;
	case RenderUpdateTaskType::TakeScreenshot:
		break;
	case RenderUpdateTaskType::ToggleRenderMode:
		break;
	case RenderUpdateTaskType::CreateActorGpuIndex:
		break;
	case RenderUpdateTaskType::AddStaticMeshInstance:
		break;
	case RenderUpdateTaskType::AddDynamicMeshInstance:
		break;
	default:
		break;
	}

	/*vx::lock_guard<vx::mutex> lck(m_updateMutex);

	if (m_doubleBuffer.memcpy(data, dataSize))
	{
		m_tasks.push_back(task);
	}
	else
	{
		puts("RenderAspect::queueUpdateTask out of memory");
		std::exit(1);
	}*/
}

void RenderAspect::queueUpdateCamera(const RenderUpdateCameraData &data)
{
	//m_taskManager->pushTask(new TaskUpdateCamera(data.position, data.quaternionRotation, &m_camera));

	/*RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::UpdateCamera;

	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_updateCameraData = data;

	m_tasks.push_back(task);*/
	updateCamera(data);
}

void RenderAspect::update()
{
	//processTasks();
}

void RenderAspect::updateProfiler(f32 dt)
{
}

void createSRView(ID3D12Resource* buffer, d3d::DescriptorHandleCpu* cpuHandle, ID3D12Device* device)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.NumElements = g_maxMeshInstances;
	desc.Buffer.StructureByteStride = sizeof(Transform);
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	cpuHandle->offset(1);

	device->CreateShaderResourceView(buffer, &desc, *cpuHandle);
}

void RenderAspect::submitCommands()
{
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Vertex) * m_vertexCount;
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(u32) * m_indexCount;

	u32 cameraBufferOffset = m_lastBuffer * d3d::getAlignedSize(sizeof(CameraBufferData), 256);
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferViewDesc{};
	cbufferViewDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress() + cameraBufferOffset;
	cbufferViewDesc.SizeInBytes = d3d::getAlignedSize(sizeof(CameraBufferData), 256);
	m_device.getDevice()->CreateConstantBufferView(&cbufferViewDesc, m_descriptorHeapBuffer->GetCPUDescriptorHandleForHeapStart());

	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	rtvHandle.offset(m_currentBuffer);

	const D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[] =
	{
		rtvHandle
	};

	ID3D12DescriptorHeap* heaps[]=
	{
		m_descriptorHeapBuffer.get()
	};

	const f32 clearColor[] = { 0.10f, 0.22f, 0.5f, 1 };

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.get(), nullptr);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	m_commandList->SetDescriptorHeaps(1, heaps);
	m_defaultRenderer.submitCommands(m_commandList);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapBuffer->GetGPUDescriptorHandleForHeapStart());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->OMSetRenderTargets(1, rtvHandles, FALSE, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	m_commandList->IASetIndexBuffer(&indexBufferView);

	for(auto &it : m_drawCommands)
	{
		m_commandList->DrawIndexedInstanced(it.indexCount, 1, it.firstIndex, 0, it.baseInstance);
	}
	//m_commandList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
	//m_commandList->ExecuteIndirect(m_commandSignature.get(), 1, m_indirectCmdBuffer.get(), 0, m_indirectCmdBuffer.get(), 256);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	//
	//m_commandList->SetGraphicsRootDescriptorTable(0, descHeap);

	auto hresult = m_commandList->Close();
	if (hresult != 0)
	{
		printf("error closing command list\n");
	}

	auto uploadCmdList = m_uploadManager.update();

	ID3D12CommandList* lists[]
	{
		uploadCmdList,
		m_commandList
	};

	m_device.executeCommandLists(lists);
}

void RenderAspect::endFrame()
{
	m_device.swapBuffer();
	m_device.waitForGpu();

	m_lastBuffer = m_currentBuffer;
	m_currentBuffer = m_device.getCurrentBackBufferIndex();
}

void RenderAspect::handleMessage(const vx::Message &evt)
{
	switch (evt.type)
	{
	case(vx::MessageType::File_Event) :
		handleFileMessage(evt);
		break;
	default:
		break;
	}
}

void RenderAspect::handleFileMessage(const vx::Message &evt)
{
	auto fileEvent = (vx::FileMessage)evt.code;

	switch (fileEvent)
	{
	case vx::FileMessage::Scene_Loaded:
	{
		auto scene = (Scene*)evt.arg2.ptr;

		loadScene(scene);

		//TaskLoadScene data;
		//data.ptr = pScene;

		//queueUpdateTask(type, (u8*)&data, sizeof(TaskLoadScene));
	}break;
	case vx::FileMessage::EditorScene_Loaded:
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
	*m = m_projectionMatrix;
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
	/*vx::lock_guard<vx::mutex> lck(m_updateMutex);
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

	VX_ASSERT(offset == backBufferSize);*/
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

void RenderAspect::loadScene(Scene* scene)
{
	auto &meshes = scene->getMeshes();
	auto meshKeys = meshes.keys();
	auto meshCount = meshes.size();

	u32 indexOffset = 0;
	u32 vertexOffset = 0;

	Vertex* vertexPtr = nullptr;
	m_vertexBuffer->Map(0, nullptr, (void**)&vertexPtr);

	u32* indexPtr = nullptr;
	m_indexBuffer->Map(0, nullptr, (void**)&indexPtr);
	for (u32 i = 0; i < meshCount; ++i)
	{
		auto meshFile = meshes[i];
		auto &mesh = meshFile->getMesh();

		auto meshVertices = mesh.getVertices();
		auto meshVertexCount = mesh.getVertexCount();
		auto meshIndices = mesh.getIndices();
		auto meshIndexCount = mesh.getIndexCount();

		for (u32 j = 0; j < meshVertexCount; ++j)
		{
			vertexPtr[j + vertexOffset].position = meshVertices[j].position;
			vertexPtr[j + vertexOffset].color = {1, 0, 0};
		}

		for (u32 j = 0; j < meshIndexCount; ++j)
		{
			indexPtr[j + indexOffset] = meshIndices[j] + vertexOffset;
		}

		MeshEntry meshEntry;
		meshEntry.indexCount = meshIndexCount;
		meshEntry.indexStart = indexOffset;

		indexOffset += meshIndexCount;
		vertexOffset += meshVertexCount;

		m_indexCount += meshIndexCount;
		m_vertexCount += meshVertexCount;

		m_meshEntries.insert(meshKeys[i], meshEntry);
	}
	m_vertexBuffer->Unmap(0, nullptr);
	m_indexBuffer->Unmap(0, nullptr);

	auto meshInstances = scene->getMeshInstances();
	auto meshInstanceCount = scene->getMeshInstanceCount();
	for (u32 i = 0; i < meshInstanceCount; ++i)
	{
		auto &meshInstance = meshInstances[i];
		auto meshEntryIt = m_meshEntries.find(meshInstance.getMeshSid());

		MeshInstanceDrawCmd cmd;
		cmd.baseInstance = i;
		cmd.baseVertex = 0;
		cmd.firstIndex = meshEntryIt->indexStart;
		cmd.indexCount = meshEntryIt->indexCount;
		cmd.instanceCount = 1;
		m_drawCommands.push_back(cmd);
	}

	/*

	auto meshSid = mehsInstances[0].getMeshSid();
	auto meshIt = meshes.find(meshSid);

	auto vertices = (*meshIt)->getMesh().getVertices();
	auto vertexCount = (*meshIt)->getMesh().getVertexCount();

	auto indices = (*meshIt)->getMesh().getIndices();
	auto indexCount = (*meshIt)->getMesh().getIndexCount();

	auto meshVertices = std::make_unique<Vertex[]>(vertexCount);
	for (u32 i = 0; i < vertexCount; ++i)
	{
		meshVertices[i].position = vertices[i].position;
		meshVertices[i].color = {1, 0, 0};
	}

	auto vertexSizeInBytes = sizeof(Vertex) * vertexCount;
	Vertex* vertexPtr = nullptr;
	m_vertexBuffer->Map(0, nullptr, (void**)&vertexPtr);
	memcpy(vertexPtr, meshVertices.get(), vertexSizeInBytes);
	m_vertexBuffer->Unmap(0, nullptr);

	auto indexCountInBytes = sizeof(u32) * indexCount;
	void* indexPtr = nullptr;
	m_indexBuffer->Map(0, nullptr, &indexPtr);
	memcpy(indexPtr, indices, indexCountInBytes);
	m_indexBuffer->Unmap(0, nullptr);

	m_indexCount += indexCount;
	m_vertexCount += vertexCount;*/

	/*D3D12_DRAW_INDEXED_ARGUMENTS cmd;
	cmd.IndexCountPerInstance = indexCount;
	cmd.InstanceCount = 1;
	cmd.StartIndexLocation = 0;
	cmd.BaseVertexLocation = 0;
	cmd.StartInstanceLocation = 0;*/

	//m_uploadManager.pushUpload((u8*)&cmd, m_indirectCmdBuffer.get(), 0, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
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