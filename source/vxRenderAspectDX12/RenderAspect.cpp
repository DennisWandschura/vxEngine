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
#include <vxEngineLib/Material.h>
#include "Vertex.h"
#include "Transform.h"
#include "CameraBufferData.h"

struct ResourceView
{
	enum class Type : u32
	{
		VertexBufferView,
		IndexBufferView
	};

	Type type;
	union
	{
		D3D12_VERTEX_BUFFER_VIEW vbv;
		D3D12_INDEX_BUFFER_VIEW ibv;
	};
};

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
	m_renderTarget(),
	m_descriptorHeapRtv(),
	m_commandAllocator(),
	m_taskManager(nullptr),
	m_resourceAspect(nullptr)
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

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 16;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descHeap.NodeMask = 1;
	if (!m_descriptorHeapBuffer.create(descHeap, &m_device))
		return false;

	D3D12_HEAP_PROPERTIES props
	{
		D3D12_HEAP_TYPE_DEFAULT,
		D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
		D3D12_MEMORY_POOL_UNKNOWN,
		0,
		0
	};

	D3D12_HEAP_DESC textureHeapDesc;
	textureHeapDesc.Alignment = 64u KBYTE;
	textureHeapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
	textureHeapDesc.Properties = props;
	textureHeapDesc.SizeInBytes = d3d::getAlignedSize(512u MBYTE, 64u KBYTE);
	if (!m_textureHeap.create(textureHeapDesc, &m_device))
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

	auto drawCmdSize = d3d::getAlignedSize(sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * g_maxMeshInstances, 256) + 256;
	drawCmdSize = d3d::getAlignedSize(drawCmdSize, 64 KBYTE);
	if (!m_bufferHeap.createResourceBuffer(drawCmdSize, 64u KBYTE, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, m_indirectCmdBuffer.getAddressOf(), &m_device))
		return false;

	auto transformBufferSize = d3d::getAlignedSize(sizeof(Transform) * g_maxMeshInstances, 64 KBYTE);
	if (!m_bufferHeap.createResourceBuffer(transformBufferSize, 64u KBYTE * 2, D3D12_RESOURCE_STATE_GENERIC_READ, m_transformBuffer.getAddressOf(), &m_device))
		return false;

	return true;
}

RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &desc)
{
	m_taskManager = desc.taskManager;
	m_resourceAspect = desc.resourceAspect;

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

	auto resolution = desc.settings->m_resolution;
	m_viewport.Height = (f32)resolution.y;
	m_viewport.Width = (f32)resolution.x;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_rectScissor.left = 0;
	m_rectScissor.top = 0;
	m_rectScissor.right = resolution.x;
	m_rectScissor.bottom = resolution.y;

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 2;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (!m_descriptorHeapRtv.create(descHeap, &m_device))
	{
		puts("Error CreateDescriptorHeap");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDsv = {};
	descHeapDsv.NumDescriptors = 1;
	descHeapDsv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descHeapDsv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (!m_descriptorHeapDsv.create(descHeapDsv, &m_device))
	{
		puts("Error CreateDescriptorHeapDsv");
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

	{
		D3D12_RESOURCE_DESC resDesc;
		resDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		resDesc.Alignment = 64 KBYTE;
		// 1024, 1024, 128
		resDesc.Width = 1024;
		resDesc.Height = 1024;
		resDesc.DepthOrArraySize = 64;
		resDesc.MipLevels = 0;
		resDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		resDesc.SampleDesc.Count = 1;
		resDesc.SampleDesc.Quality = 0;
		resDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		resDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		if (!m_textureHeap.createResource(resDesc, 0, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, m_textureBuffer.getAddressOf(), &m_device))
			return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	D3D12_INDIRECT_ARGUMENT_DESC argumentDescs[1] = {};
	argumentDescs[0].Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

	D3D12_COMMAND_SIGNATURE_DESC cmdSigDesc;
	cmdSigDesc.ByteStride = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS);
	cmdSigDesc.NodeMask = 0;
	cmdSigDesc.NumArgumentDescs = 1;
	cmdSigDesc.pArgumentDescs = argumentDescs;
	hresult = device->CreateCommandSignature(&cmdSigDesc, nullptr, IID_PPV_ARGS(m_commandSignature.getAddressOf()));


	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	hresult = m_device.getDevice()->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), 
		D3D12_HEAP_FLAG_NONE, 
		&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, resolution.x, resolution.y, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthOptimizedClearValue,
		IID_PPV_ARGS(m_depthTexture.getAddressOf())
		);
	if (hresult != 0)
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if(!m_meshManager.initialize(g_maxVertexCount, g_maxIndexCount, g_maxMeshInstances, &m_device, desc.pAllocator))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	createCbvCamera();
	updateSrvTransform(0);

	return RenderAspectInitializeError::OK;
}

void RenderAspect::shutdown(void* hwnd)
{
	m_device.waitForGpu();

	m_meshManager.shutdown();

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
	m_uploadManager.pushUpload((u8*)&bufferData, m_constantBuffer.get(), 0, sizeof(CameraBufferData), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
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

void RenderAspect::createCbvCamera()
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferViewDesc{};
	cbufferViewDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbufferViewDesc.SizeInBytes = d3d::getAlignedSize(sizeof(CameraBufferData), 256);

	auto descriptorHeapBufferHandle = m_descriptorHeapBuffer.getHandleCpu();
	m_device.getDevice()->CreateConstantBufferView(&cbufferViewDesc, descriptorHeapBufferHandle);
}

void RenderAspect::updateSrvTransform(u32 instanceCount)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = instanceCount;
	srvDesc.Buffer.StructureByteStride = sizeof(Transform);

	auto descriptorHeapBufferHandle = m_descriptorHeapBuffer.getHandleCpu();
	descriptorHeapBufferHandle.offset(1);
	m_device.getDevice()->CreateShaderResourceView(m_transformBuffer.get(), &srvDesc, descriptorHeapBufferHandle);
}

void RenderAspect::submitCommands()
{
	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	rtvHandle.offset(m_currentBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1];
	rtvHandles[0] = rtvHandle;

	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandles[1];
	dsvHandles[0] = m_descriptorHeapDsv.getHandleCpu();

	ID3D12DescriptorHeap* heaps[] =
	{
		m_descriptorHeapBuffer.get()
	};

	const f32 clearColor[] = { 0.10f, 0.22f, 0.5f, 1 };

	D3D12_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
	depthViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthViewDesc.Texture2D.MipSlice = 0;
	m_device.getDevice()->CreateDepthStencilView(m_depthTexture.get(), &depthViewDesc, m_descriptorHeapDsv.getHandleCpu());

	m_commandAllocator->Reset();
	m_commandList->Reset(m_commandAllocator.get(), nullptr);

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	m_commandList->SetDescriptorHeaps(1, heaps);
	m_defaultRenderer.submitCommands(m_commandList);
	m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapBuffer->GetGPUDescriptorHandleForHeapStart());

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->OMSetRenderTargets(1, rtvHandles, FALSE, dsvHandles);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandles[0], D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	if (!m_drawCommands.empty())
	{
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = m_resourceViews.find(vx::make_sid("meshVertexBufferView"))->vbv;
		vertexBufferViews[1] = m_resourceViews.find(vx::make_sid("meshDrawIdBufferView"))->vbv;
		auto indexBufferView = m_resourceViews.find(vx::make_sid("meshIndexBufferView"))->ibv;

		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 2, vertexBufferViews);
		m_commandList->IASetIndexBuffer(&indexBufferView);

		auto cmdCount = m_drawCommands.size();
		/*for (auto &it : m_drawCommands)
		{
			m_commandList->DrawIndexedInstanced(it.indexCount, it.instanceCount, it.firstIndex, 0, it.baseInstance);
		}*/

		auto countOffset = d3d::getAlignedSize(sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * g_maxMeshInstances, 256);
		m_commandList->ExecuteIndirect(m_commandSignature.get(), cmdCount, m_indirectCmdBuffer.get(), 0, m_indirectCmdBuffer.get(), countOffset);
	}

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

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
	auto meshInstances = scene->getMeshInstances();
	auto meshInstanceCount = scene->getMeshInstanceCount();

	for (u32 i = 0; i < meshInstanceCount; ++i)
	{
		auto &meshInstance = meshInstances[i];
		auto material = meshInstance.getMaterial();

		auto diffuseTextureSid = material->m_textureSid[0];

		DrawIndexedCommand cmd;
		m_meshManager.addMeshInstance(meshInstance, m_resourceAspect, &cmd);

		m_drawCommands.push_back(cmd);

		auto meshTransform = meshInstance.getTransform();

		Transform transform;
		transform.translation = vx::float4(meshTransform.m_translation, 1);

		auto transformOffset = sizeof(Transform) * cmd.baseInstance;
		m_uploadManager.pushUpload((u8*)&transform, m_transformBuffer.get(), transformOffset, sizeof(Transform), D3D12_RESOURCE_STATE_GENERIC_READ);

		auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * cmd.baseInstance;

		D3D12_DRAW_INDEXED_ARGUMENTS cmdArgs;
		cmdArgs.BaseVertexLocation = cmd.baseVertex;
		cmdArgs.IndexCountPerInstance = cmd.indexCount;
		cmdArgs.InstanceCount = cmd.instanceCount;
		cmdArgs.StartIndexLocation = cmd.firstIndex;
		cmdArgs.StartInstanceLocation = cmd.baseInstance;
		m_uploadManager.pushUpload((u8*)&cmdArgs, m_indirectCmdBuffer.get(), cmdOffset, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);
	//	m_indirectCmdBuffer;
	}
	auto instanceCount = m_meshManager.getInstanceCount();

	auto countOffset = d3d::getAlignedSize(sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * g_maxMeshInstances, 256);
	u32 count = instanceCount;
	m_uploadManager.pushUpload((u8*)&count, m_indirectCmdBuffer.get(), countOffset, sizeof(u32), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	ResourceView meshVertexBufferView;
	meshVertexBufferView.type = ResourceView::Type::VertexBufferView;
	meshVertexBufferView.vbv = m_meshManager.getVertexBufferView();;
	m_resourceViews.insert(vx::make_sid("meshVertexBufferView"), meshVertexBufferView);

	ResourceView meshDrawIdBufferView;
	meshDrawIdBufferView.type = ResourceView::Type::VertexBufferView;
	meshDrawIdBufferView.vbv = m_meshManager.getDrawIdBufferView();
	m_resourceViews.insert(vx::make_sid("meshDrawIdBufferView"), meshDrawIdBufferView);

	ResourceView meshIndexBufferView;
	meshIndexBufferView.type = ResourceView::Type::IndexBufferView;
	meshIndexBufferView.ibv = m_meshManager.getIndexBufferView();
	m_resourceViews.insert(vx::make_sid("meshIndexBufferView"), meshIndexBufferView);

	
	updateSrvTransform(instanceCount);
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