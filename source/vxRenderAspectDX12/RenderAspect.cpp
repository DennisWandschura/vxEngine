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
#include <vxEngineLib/Logfile.h>
#include <d3d12sdklayers.h>
#include <vxEngineLib/CreateDynamicMeshData.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/CreateActorData.h>
#include <vxLib/ScopeGuard.h>
#include "GpuLight.h"
#include <vxEngineLib/Light.h>

#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/GpuFunctions.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Material.h>
#include "Vertex.h"
#include <vxEngineLib/Transform.h>
#include "GpuCameraBufferData.h"

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

const u32 g_cbufferOffsetLightCount = d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;

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

	static const GUID IID_ID3D12InfoQueue = { 0x0742a90b, 0xc387, 0x483f,{ 0xb9, 0x46, 0x30, 0xa7, 0xe4, 0xe6, 0x14, 0x58 } };
	//0x0742a90b, 0xc387, 0x483f, 0xb9, 0x46, 0x30, 0xa7, 0xe4, 0xe6, 0x14, 0x58
}

RenderAspect::RenderAspect()
	:m_device(),
	m_commandList(nullptr),
	m_renderTarget(),
	m_descriptorHeapRtv(),
	m_commandAllocator(),
	m_taskManager(nullptr),
	m_resourceAspect(nullptr),
	m_msgManager(nullptr)
{

}

RenderAspect::~RenderAspect()
{
}

bool RenderAspect::createCommandList()
{
	auto device = m_device.getDevice();
	auto result = device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(&m_commandList));
	if (result != 0)
	{
		puts("error creating command list");
		return false;
	}

	m_commandList->Close();

	m_commandListDrawMesh = nullptr;
	result = device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(&m_commandListDrawMesh));
	if (result != 0)
	{
		puts("error creating command list");
		return false;
	}
	m_commandListDrawMesh->Close();

	return true;
}

bool RenderAspect::createHeaps()
{
	if (!m_bufferHeap.createBufferHeap(64u KBYTE * 5, D3D12_HEAP_TYPE_DEFAULT, &m_device))
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
	u32 offset = 0;
	const u32 cbufferSize = 64u KBYTE;
	if (!m_bufferHeap.createResourceBuffer(cbufferSize, offset, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, m_constantBuffer.getAddressOf(), &m_device))
		return false;

	offset += cbufferSize;

	auto drawCmdSize = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size + 256;
	drawCmdSize = d3d::getAlignedSize(drawCmdSize, 64llu KBYTE);
	if (!m_bufferHeap.createResourceBuffer(drawCmdSize, offset, D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, m_indirectCmdBuffer.getAddressOf(), &m_device))
		return false;

	offset += drawCmdSize;

	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances * 2, 64 KBYTE>::size;
	if (!m_bufferHeap.createResourceBuffer(transformBufferSize, offset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, m_transformBuffer.getAddressOf(), &m_device))
		return false;

	offset += transformBufferSize;

	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;

	if (!m_bufferHeap.createResourceBuffer(materialBufferSize, offset, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, m_materialBuffer.getAddressOf(), &m_device))
		return false;

	offset += materialBufferSize;

	if (!m_bufferHeap.createResourceBuffer(64 KBYTE, offset, D3D12_RESOURCE_STATE_COMMON, m_lightBuffer.getAddressOf(), &m_device))
		return false;
	

	return true;
}

RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &desc)
{
	auto errorlog = desc.errorlog;

	SCOPE_EXIT
	{
		printDebugMessages();
	};

	m_taskManager = desc.taskManager;
	m_resourceAspect = desc.resourceAspect;
	m_msgManager = desc.msgManager;

	auto screenAspect = (f32)desc.settings->m_resolution.x / (f32)desc.settings->m_resolution.y;
	auto fovRad = vx::degToRad(desc.settings->m_fovDeg);
	m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(fovRad, screenAspect, desc.settings->m_zNear, desc.settings->m_zFar);

	const u32 scratchAllocSize = 10 KBYTE;
	auto scratchAllocPtr = desc.pAllocator->allocate(scratchAllocSize);
	VX_ASSERT(scratchAllocPtr);
	m_scratchAllocator = vx::StackAllocator(scratchAllocPtr, scratchAllocSize);

	const auto doubleBufferSizeInBytes = 5 KBYTE;

	auto debugMode = desc.settings->m_renderDebug;

	if (debugMode)
	{
		auto hresult = D3D12GetDebugInterface(IID_PPV_ARGS(m_debug.getAddressOf()));
		if (hresult != 0)
		{
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}
		m_debug->EnableDebugLayer();
	}

	if (!m_device.initialize(*desc.window))
	{
		errorlog->append("error initializing device\n");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	auto device = m_device.getDevice();

	if (debugMode)
	{
		auto hresult = device->QueryInterface(IID_PPV_ARGS(m_infoQueue.getAddressOf()));
		if (hresult != 0)
		{
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}

		/*D3D12_MESSAGE_CATEGORY allocCategories[] = 
		{
			D3D12_MESSAGE_CATEGORY::
		};


		D3D12_INFO_QUEUE_FILTER filter;
		filter.AllowList.NumCategories = ;
		filter.AllowList.pCategoryList = ;
		filter.AllowList.NumSeverities;
		filter.AllowList.pSeverityList;
		filter.AllowList.NumIDs;
		filter.AllowList.pIDList;
		filter.DenyList = ;

		m_infoQueue->AddStorageFilterEntries();*/
		D3D12_INFO_QUEUE_FILTER filter;
		memset(&filter, 0, sizeof(filter));

		D3D12_MESSAGE_SEVERITY severities[] = 
		{
			D3D12_MESSAGE_SEVERITY_CORRUPTION,
			D3D12_MESSAGE_SEVERITY_ERROR
		};

		filter.AllowList.NumSeverities = 2;
		filter.AllowList.pSeverityList = severities;

		//hresult = m_infoQueue->AddRetrievalFilterEntries(&filter);
	}

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
		errorlog->append("Error CreateDescriptorHeap");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeapDsv = {};
	descHeapDsv.NumDescriptors = 1;
	descHeapDsv.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	descHeapDsv.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (!m_descriptorHeapDsv.create(descHeapDsv, &m_device))
	{
		errorlog->append("Error CreateDescriptorHeapDsv");
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

	if(!m_gbufferRenderer.initialize(resolution, &m_device, &m_shaderManager))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_uploadManager.initialize(&m_device))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_drawQuadRenderer.initialize(device, &m_shaderManager))
	{
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

	u32 srgbCount = 32;
	u32 rgbCount = 64;
	if (!m_materialManager.initialize(vx::uint2(1024), srgbCount, rgbCount, &m_device))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	createCbvCamera();
	createSrvTextures(srgbCount, rgbCount);
	updateSrvTransform(1);
	updateSrvMaterial(1);

	ResourceView meshVertexBufferView;
	meshVertexBufferView.type = ResourceView::Type::VertexBufferView;
	meshVertexBufferView.vbv = m_meshManager.getVertexBufferView();
	m_resourceViews.insert(vx::make_sid("meshVertexBufferView"), meshVertexBufferView);

	ResourceView meshDrawIdBufferView;
	meshDrawIdBufferView.type = ResourceView::Type::VertexBufferView;
	meshDrawIdBufferView.vbv = m_meshManager.getDrawIdBufferView();
	m_resourceViews.insert(vx::make_sid("meshDrawIdBufferView"), meshDrawIdBufferView);

	ResourceView meshIndexBufferView;
	meshIndexBufferView.type = ResourceView::Type::IndexBufferView;
	meshIndexBufferView.ibv = m_meshManager.getIndexBufferView();
	m_resourceViews.insert(vx::make_sid("meshIndexBufferView"), meshIndexBufferView);

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

bool RenderAspect::initializeProfiler(Logfile* errorlog)
{
	return true;
}

void RenderAspect::makeCurrent(bool b)
{
}

void RenderAspect::printDebugMessages()
{
	auto msgCount = m_infoQueue->GetNumStoredMessagesAllowedByRetrievalFilter();
	auto marker = m_scratchAllocator.getMarker();
	for (u64 i = 0; i < msgCount; ++i)
	{
		size_t msgSize = 0;
		auto hresult = m_infoQueue->GetMessage(i, nullptr, &msgSize);

		D3D12_MESSAGE* msg = (D3D12_MESSAGE*)m_scratchAllocator.allocate(msgSize, 8);
		if (msg)
		{
			hresult = m_infoQueue->GetMessage(i, msg, &msgSize);
			printf("%s\n", msg->pDescription);
		}
	}
	m_infoQueue->ClearStoredMessages();

	m_scratchAllocator.clear(marker);
}

void RenderAspect::updateCamera(const RenderUpdateCameraData &data)
{
	m_camera.setPosition(data.position);
	m_camera.setRotation(data.quaternionRotation);

	vx::mat4 viewMatrix;
	m_camera.getViewMatrix(&viewMatrix);

	GpuCameraBufferData bufferData;
	bufferData.position = m_camera.getPosition();
	bufferData.viewMatrix = viewMatrix;
	bufferData.pvMatrixPrev = bufferData.pvMatrix;
	bufferData.pvMatrix = m_projectionMatrix * viewMatrix;

	u32 offset = m_currentBuffer * d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;
	m_uploadManager.pushUploadBuffer((u8*)&bufferData, m_constantBuffer.get(), 0, sizeof(GpuCameraBufferData), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTaskType type, const u8* data, u32 dataSize)
{
	switch (type)
	{
	case RenderUpdateTaskType::UpdateCamera:
		break;
	case RenderUpdateTaskType::UpdateDynamicTransforms:
	{
		u32 offset = 0;
		taskUpdateDynamicTransforms(data, &offset);
	}break;
	case RenderUpdateTaskType::UpdateText:
		break;
	case RenderUpdateTaskType::LoadScene:
		break;
	case RenderUpdateTaskType::TakeScreenshot:
		break;
	case RenderUpdateTaskType::ToggleRenderMode:
		break;
	case RenderUpdateTaskType::CreateActorGpuIndex:
	{
		u32 offset = 0;
		taskCreateActorGpuIndex(data, &offset);
	}break;
	case RenderUpdateTaskType::AddStaticMeshInstance:
	{
		u32 offset = 0;
		taskAddStaticMeshInstance(data, &offset);
	}break;
	case RenderUpdateTaskType::AddDynamicMeshInstance:
	{
		u32 offset = 0;
		taskAddDynamicMeshInstance(data, &offset);
	}break;
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
	printDebugMessages();
}

void RenderAspect::updateProfiler(f32 dt)
{
}

void RenderAspect::createCbvCamera()
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferViewDesc{};
	cbufferViewDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbufferViewDesc.SizeInBytes = d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;

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
	srvDesc.Buffer.StructureByteStride = sizeof(vx::TransformGpu);

	auto descriptorHeapBufferHandle = m_descriptorHeapBuffer.getHandleCpu();
	descriptorHeapBufferHandle.offset(1);
	m_device.getDevice()->CreateShaderResourceView(m_transformBuffer.get(), &srvDesc, descriptorHeapBufferHandle);
}

void RenderAspect::updateSrvMaterial(u32 instanceCount)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = instanceCount;
	srvDesc.Buffer.StructureByteStride = sizeof(u32);

	auto descriptorHeapBufferHandle = m_descriptorHeapBuffer.getHandleCpu();
	descriptorHeapBufferHandle.offset(2);

	m_device.getDevice()->CreateShaderResourceView(m_materialBuffer.get(), &srvDesc, descriptorHeapBufferHandle);
}

void RenderAspect::createSrvTextures(u32 srgbCount, u32 rgbCount)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_BC7_UNORM_SRGB;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2DArray.ArraySize = srgbCount;
	srvDesc.Texture2DArray.FirstArraySlice = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.MostDetailedMip = 0;
	//srvDesc.Texture2DArray.PlaneSlice = ;
	//srvDesc.Texture2DArray.ResourceMinLODClamp = ;

	auto descriptorHeapBufferHandle = m_descriptorHeapBuffer.getHandleCpu();
	descriptorHeapBufferHandle.offset(3);
	m_device.getDevice()->CreateShaderResourceView(m_materialManager.getTextureBufferSrgba().get(), &srvDesc, descriptorHeapBufferHandle);

	descriptorHeapBufferHandle.offset(1);
	srvDesc.Format = DXGI_FORMAT_BC7_UNORM;
	srvDesc.Texture2DArray.ArraySize = rgbCount;
	m_device.getDevice()->CreateShaderResourceView(m_materialManager.getTextureBufferRgba().get(), &srvDesc, descriptorHeapBufferHandle);
}

void RenderAspect::drawScreenQuadGBuffer(ID3D12GraphicsCommandList* cmdList)
{
	const f32 clearColor[] = { 0.10f, 0.22f, 0.5f, 1 };
	auto rtvHandle = m_descriptorHeapRtv.getHandleCpu();
	rtvHandle.offset(m_currentBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[1];
	rtvHandles[0] = rtvHandle;

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_rectScissor);

	m_drawQuadRenderer.setPipelineState(m_commandList);
	m_gbufferRenderer.bindSrvBegin(m_commandList);
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer].get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->OMSetRenderTargets(1, rtvHandles, FALSE, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	m_drawQuadRenderer.submitCommands(m_commandList);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer].get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_gbufferRenderer.bindSrvEnd(m_commandList);
}

void RenderAspect::submitCommands()
{
	ID3D12DescriptorHeap* heaps[] =
	{
		m_descriptorHeapBuffer.get()
	};

	m_commandAllocator->Reset();
	m_commandListDrawMesh->Reset(m_commandAllocator.get(), nullptr);

	m_commandListDrawMesh->RSSetViewports(1, &m_viewport);
	m_commandListDrawMesh->RSSetScissorRects(1, &m_rectScissor);

	m_gbufferRenderer.submitCommands(m_commandListDrawMesh);
	m_commandListDrawMesh->SetDescriptorHeaps(1, heaps);
	m_commandListDrawMesh->SetGraphicsRootDescriptorTable(0, m_descriptorHeapBuffer->GetGPUDescriptorHandleForHeapStart());
	m_commandListDrawMesh->SetGraphicsRootDescriptorTable(1, m_descriptorHeapBuffer->GetGPUDescriptorHandleForHeapStart());
	m_commandListDrawMesh->SetGraphicsRootDescriptorTable(2, m_descriptorHeapBuffer->GetGPUDescriptorHandleForHeapStart());

	if (!m_drawCommands.empty())
	{
		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = m_resourceViews.find(vx::make_sid("meshVertexBufferView"))->vbv;
		vertexBufferViews[1] = m_resourceViews.find(vx::make_sid("meshDrawIdBufferView"))->vbv;
		auto indexBufferView = m_resourceViews.find(vx::make_sid("meshIndexBufferView"))->ibv;

		m_commandListDrawMesh->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandListDrawMesh->IASetVertexBuffers(0, 2, vertexBufferViews);
		m_commandListDrawMesh->IASetIndexBuffer(&indexBufferView);

		auto cmdCount = m_drawCommands.size();
		const auto countOffset = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size;
		m_commandListDrawMesh->ExecuteIndirect(m_commandSignature.get(), cmdCount, m_indirectCmdBuffer.get(), 0, m_indirectCmdBuffer.get(), countOffset);
	}
	m_commandListDrawMesh->Close();

	m_commandList->Reset(m_commandAllocator.get(), nullptr);
	drawScreenQuadGBuffer(m_commandList);
	auto hresult = m_commandList->Close();
	if (hresult != 0)
	{
		printf("error closing command list\n");
	}

	auto uploadCmdList = m_uploadManager.update();

	ID3D12CommandList* lists[]
	{
		uploadCmdList,
		m_commandListDrawMesh,
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
}

void RenderAspect::taskUpdateText(const u8* p, u32* offset)
{
	auto data = (RenderUpdateTextData*)p;

	//m_textRenderer->pushEntry(std::move(data->text), data->position, data->color);

	*offset += sizeof(RenderUpdateTextData);
}

void RenderAspect::taskTakeScreenshot()
{
}

void RenderAspect::updateTransform(const vx::Transform &meshTransform, u32 index)
{
	auto meshRotation = vx::loadFloat4(meshTransform.m_qRotation);

	vx::TransformGpu gpuTransform;
	gpuTransform.translation = meshTransform.m_translation;
	gpuTransform.scaling = 1.0f;
	gpuTransform.packedQRotation = GpuFunctions::packQRotation(meshRotation);
	updateTransform(gpuTransform, index);
}

void RenderAspect::updateTransform(const vx::TransformGpu &transform, u32 index)
{
	auto transformOffset = sizeof(vx::TransformGpu) * index;
	m_uploadManager.pushUploadBuffer((u8*)&transform, m_transformBuffer.get(), transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderAspect::addMeshInstance(const MeshInstance &meshInstance, u32* gpuIndex)
{
	auto material = meshInstance.getMaterial();
	u32 materialIndex = 0;
	u32 materialSlices = 0;
	m_materialManager.addMaterial(material, m_resourceAspect, &m_uploadManager, &materialIndex, &materialSlices);

	DrawIndexedCommand cmd;
	m_meshManager.addMeshInstance(meshInstance, materialIndex, m_resourceAspect, &cmd);
	m_drawCommands.push_back(cmd);

	auto meshTransform = meshInstance.getTransform();
	updateTransform(meshTransform, cmd.baseInstance);

	auto materialOffset = sizeof(u32) * materialIndex;
	m_uploadManager.pushUploadBuffer((u8*)&materialSlices, m_materialBuffer.get(), materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * cmd.baseInstance;

	D3D12_DRAW_INDEXED_ARGUMENTS cmdArgs;
	cmdArgs.BaseVertexLocation = cmd.baseVertex;
	cmdArgs.IndexCountPerInstance = cmd.indexCount;
	cmdArgs.InstanceCount = cmd.instanceCount;
	cmdArgs.StartIndexLocation = cmd.firstIndex;
	cmdArgs.StartInstanceLocation = cmd.baseInstance;
	m_uploadManager.pushUploadBuffer((u8*)&cmdArgs, m_indirectCmdBuffer.get(), cmdOffset, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	*gpuIndex = cmd.baseInstance;

	auto instanceCount = m_meshManager.getInstanceCount();

	const auto countOffset = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256llu>::size;
	u32 count = instanceCount;
	m_uploadManager.pushUploadBuffer((u8*)&count, m_indirectCmdBuffer.get(), countOffset, sizeof(u32), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	auto meshVertexBufferViewIter = m_resourceViews.find(vx::make_sid("meshVertexBufferView"));
	meshVertexBufferViewIter->vbv = m_meshManager.getVertexBufferView();;

	auto meshDrawIdBufferViewIter = m_resourceViews.find(vx::make_sid("meshDrawIdBufferView"));
	meshDrawIdBufferViewIter->vbv = m_meshManager.getDrawIdBufferView();;

	auto meshIndexBufferViewIter = m_resourceViews.find(vx::make_sid("meshIndexBufferView"));
	meshIndexBufferViewIter->ibv = m_meshManager.getIndexBufferView();

	updateSrvTransform(instanceCount);
	updateSrvMaterial(instanceCount);

	//printf("%u\n", instanceCount);
}

void RenderAspect::updateLights(const Light* lights, u32 count)
{
	auto gpuLights = std::make_unique<GpuLight[]>(count);
	for (u32 i = 0; i < count; ++i)
	{
		gpuLights[i].position = vx::float4(lights[i].m_position, 1);
		gpuLights[i].falloff = lights[i].m_falloff;
		gpuLights[i].lumen = lights[i].m_lumen;
	} 

	m_uploadManager.pushUploadBuffer((u8*)&count, m_lightBuffer.get(), 0, sizeof(GpuLight) * count, D3D12_RESOURCE_STATE_COMMON);

	m_uploadManager.pushUploadBuffer((u8*)&count, m_constantBuffer.get(), g_cbufferOffsetLightCount, sizeof(u32), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

void RenderAspect::loadScene(Scene* scene)
{
	auto lights = scene->getLights();
	auto lightCount = scene->getLightCount();

	updateLights(lights, lightCount);

	/*auto meshInstances = scene->getMeshInstances();
	auto meshInstanceCount = scene->getMeshInstanceCount();

	for (u32 i = 0; i < meshInstanceCount; ++i)
	{
		auto &meshInstance = meshInstances[i];

		u32 gpuIndex = 0;
		addMeshInstance(meshInstance, &gpuIndex);
	}*/
}

void RenderAspect::taskToggleRenderMode()
{
}

void RenderAspect::taskCreateActorGpuIndex(const u8* p, u32* offset)
{
	std::size_t* address = (std::size_t*)p;

	CreateActorData* data = (CreateActorData*)(*address);
	auto actorTransform = data->getTransform();
	auto qRotation = vx::loadFloat4(actorTransform.m_qRotation);

	u32 materialIndex = 0;
	u32 materialSlices = 0;
	m_materialManager.addMaterial(data->getMaterialSid(), m_resourceAspect, &m_uploadManager, &materialIndex, &materialSlices);

	DrawIndexedCommand cmd;
	m_meshManager.addMeshInstance(data->getActorSid(), data->getMeshSid(), materialIndex, m_resourceAspect, &cmd);
	data->setGpu(cmd.baseInstance);

	vx::Message e;
	e.arg1.ptr = data;
	e.code = (u32)IngameMessage::Gpu_AddedActor;
	e.type = vx::MessageType::Ingame_Event;

	m_msgManager->addMessage(e);

	updateTransform(actorTransform, cmd.baseInstance);

	auto materialOffset = sizeof(u32) * materialIndex;
	m_uploadManager.pushUploadBuffer((u8*)&materialSlices, m_materialBuffer.get(), materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * cmd.baseInstance;

	D3D12_DRAW_INDEXED_ARGUMENTS cmdArgs;
	cmdArgs.BaseVertexLocation = cmd.baseVertex;
	cmdArgs.IndexCountPerInstance = cmd.indexCount;
	cmdArgs.InstanceCount = cmd.instanceCount;
	cmdArgs.StartIndexLocation = cmd.firstIndex;
	cmdArgs.StartInstanceLocation = cmd.baseInstance;
	m_uploadManager.pushUploadBuffer((u8*)&cmdArgs, m_indirectCmdBuffer.get(), cmdOffset, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	*offset += sizeof(std::size_t);
}

void RenderAspect::taskUpdateDynamicTransforms(const u8* p, u32* offset)
{
	auto data = (RenderUpdateDataTransforms*)p;
	u32 count = data->count;

	auto transforms = (vx::TransformGpu*)(data + 1);
	auto indices = (u32*)(p + sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count);

	for (u32 i = 0; i < count; ++i)
	{
		auto index = indices[i];
		auto &transform = transforms[i];

		updateTransform(transform, index);
	}

	*offset += sizeof(RenderUpdateDataTransforms) + (sizeof(vx::TransformGpu) + sizeof(u32)) * count;
}

void RenderAspect::taskAddStaticMeshInstance(const u8* p, u32* offset)
{
	RenderUpdateTaskAddStaticMeshData* data = (RenderUpdateTaskAddStaticMeshData*)p;

	auto &instance = *data->instance;

	u32 gpuIndex = 0;
	addMeshInstance(instance, &gpuIndex);

	*offset += sizeof(RenderUpdateTaskAddStaticMeshData);
}

void RenderAspect::taskAddDynamicMeshInstance(const u8* p, u32* offset)
{
	std::size_t* address = (std::size_t*)p;

	CreateDynamicMeshData* data = (CreateDynamicMeshData*)(*address);
	auto &instance = *data->m_meshInstance;

	u32 gpuIndex = 0;
	addMeshInstance(instance, &gpuIndex);

	data->m_gpuIndex = gpuIndex;
	data->increment();

	vx::Message e;
	e.code = (u32)IngameMessage::Gpu_AddedDynamicMesh;
	e.type = vx::MessageType::Ingame_Event;
	e.arg1.ptr = data;

	m_msgManager->addMessage(e);

	*offset += sizeof(std::size_t);
}