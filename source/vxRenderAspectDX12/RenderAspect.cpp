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
#include "TaskUpdateCamera.h"
#include <vxEngineLib/TaskManager.h>
#include "TaskUploadGeometry.h"
#include <d3dcompiler.h>
#include <d3d12sdklayers.h>

#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/MeshFile.h>

struct Vertex
{
	vx::float3 position;
	int index;
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
const u32 g_maxVertexCount{ 10000 };
const u32 g_maxIndexCount{ 20000 };
const u32 g_maxMeshInstances{ 128 };

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
	m_commandAllocator(),
	m_uploadHeap(),
	m_geometryUploadBuffer(),
	m_meshIndexOffset(0),
	m_meshEntries(),
	m_taskManager(nullptr),
	m_vertexCount(0),
	m_indexCount(0)
{

}

RenderAspect::~RenderAspect()
{

}

bool RenderAspect::createCommandList()
{
	auto result = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(&m_commandList));
	if (result != 0)
	{
		puts("error creating command list");
		return false;
	}

	m_commandAllocator->Reset();

	m_commandList->Reset(m_commandAllocator.get(), nullptr);

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

bool RenderAspect::createHeaps()
{
	auto uploadHeapSize = Buffer::calculateAllocSize(sizeof(Vertex) * g_maxVertexCount) + Buffer::calculateAllocSize(sizeof(u32) * g_maxIndexCount);

	if (!m_uploadHeap.create(uploadHeapSize, m_device))
	{
		return false;
	}

	auto cameraBufferSize = Buffer::calculateAllocSize(sizeof(CameraBufferData));
	auto transformBufferSize = Buffer::calculateAllocSize(sizeof(Transform) * g_maxMeshInstances);
	auto defaultBufferHeapSize = Buffer::calculateAllocSize(cameraBufferSize + transformBufferSize);
	if (!m_defaultBufferHeap.create(defaultBufferHeapSize, m_device))
	{
		return false;
	}

	auto geometryHeapSize = Buffer::calculateAllocSize(sizeof(Vertex) * g_maxVertexCount) + Buffer::calculateAllocSize(sizeof(u32) * g_maxIndexCount);
	if (!m_defaultGeometryHeap.create(geometryHeapSize, m_device))
	{
		return false;
	}

	return true;
}

bool RenderAspect::createMeshBuffers()
{
	m_geometryUploadBuffer = m_uploadHeap.createBuffer(sizeof(Vertex) * g_maxVertexCount + sizeof(u32) * g_maxIndexCount, D3D12_RESOURCE_FLAG_NONE, m_device);

	if (!m_geometryUploadBuffer.isValid())
		return false;

	D3D12_RESOURCE_DESC desc;
	desc.Dimension = D3D12_RESOURCE_DIMENSION::D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 64 KBYTE;
	desc.Width = Buffer::calculateAllocSize(sizeof(Vertex) * g_maxVertexCount);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	auto hresult = m_device->CreatePlacedResource(m_defaultGeometryHeap.get(), 0, &desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_vertexBuffer));
	if (hresult != 0)
		return false;

	auto offset = desc.Width;
	desc.Width = Buffer::calculateAllocSize(sizeof(u32) * g_maxIndexCount);

	hresult = m_device->CreatePlacedResource(m_defaultGeometryHeap.get(), offset, &desc, D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_indexBuffer));
	if (hresult != 0)
		return false;

	return true;
}

bool RenderAspect::loadShaders()
{
	ID3DBlob* vsShader = nullptr;
	auto hresult = D3DReadFileToBlob(L"../../lib/VertexShader.cso", &vsShader);
	if (hresult != 0)
		return false;

	ID3DBlob* psShader = nullptr;
	hresult = D3DReadFileToBlob(L"../../lib/PixelShader.cso", &psShader);
	if (hresult != 0)
		return false;

	m_shaders.insert(vx::make_sid("VertexShader"), vsShader);
	m_shaders.insert(vx::make_sid("PixelShader"), psShader);

	return true;
}

bool RenderAspect::createRootSignature()
{
	/*
	D3D12_DESCRIPTOR_RANGE_TYPE RangeType;
	UINT NumDescriptors;
	UINT BaseShaderRegister;
	UINT RegisterSpace;
	UINT OffsetInDescriptorsFromTableStart;
	*/
	D3D12_DESCRIPTOR_RANGE range0;
	range0.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range0.NumDescriptors = 1;
	range0.BaseShaderRegister = 0;
	range0.RegisterSpace = 0;
	range0.OffsetInDescriptorsFromTableStart = 0;

	D3D12_DESCRIPTOR_RANGE range1;
	range1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range1.NumDescriptors = 1;
	range1.BaseShaderRegister = 0;
	range1.RegisterSpace = 0;
	range1.OffsetInDescriptorsFromTableStart = 1;

	const D3D12_DESCRIPTOR_RANGE ranges[]={ range0, range1};

	/*
	UINT NumDescriptorRanges;
	_Field_size_full_(NumDescriptorRanges)  const D3D12_DESCRIPTOR_RANGE *pDescriptorRanges;
	*/
	D3D12_ROOT_DESCRIPTOR_TABLE table;
	table.NumDescriptorRanges = 2;
	table.pDescriptorRanges = ranges;

	D3D12_ROOT_PARAMETER param;
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.DescriptorTable = table;
	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

	D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // Only the input assembler stage needs access to the constant buffer.
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

	D3D12_ROOT_SIGNATURE_DESC desc;
	desc.NumParameters = 1;
	desc.pParameters = &param;
	desc.NumStaticSamplers = 0;
	desc.pStaticSamplers = nullptr;
	desc.Flags = rootSignatureFlags;

	ID3DBlob* blob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	auto hresult = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &errorBlob);
	if (hresult != 0)
		return false;

	ID3D12RootSignature* rootSignature = nullptr;
	hresult = m_device->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (hresult != 0)
		return false;

	m_rootSignatures.insert(vx::make_sid("defaultShader"), rootSignature);

	return true;
}

bool RenderAspect::createPipelineState()
{
	auto rootSig = *m_rootSignatures.find(vx::make_sid("defaultShader"));
	auto vs = *m_shaders.find(vx::make_sid("VertexShader"));
	auto ps = *m_shaders.find(vx::make_sid("PixelShader"));

	D3D12_INPUT_ELEMENT_DESC inputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32_SINT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPso = {};
	ZeroMemory(&descPso, sizeof(descPso));
	descPso.InputLayout = { inputLayout, 2 };
	descPso.pRootSignature = rootSig;
	descPso.VS = { reinterpret_cast<BYTE*>(vs->GetBufferPointer()), vs->GetBufferSize() };
	descPso.PS = { reinterpret_cast<BYTE*>(ps->GetBufferPointer()), ps->GetBufferSize() };
	descPso.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPso.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPso.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPso.SampleMask = UINT_MAX;
	descPso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPso.NumRenderTargets = 1;
	descPso.RTVFormats[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
	descPso.SampleDesc.Count = 1;

	ID3D12PipelineState* pipelineState = nullptr;
	auto hresult = m_device->CreateGraphicsPipelineState(&descPso, IID_PPV_ARGS(&pipelineState));
	if (hresult != 0)
	{
		return false;
	}

	m_pipelineStates.insert(vx::make_sid("defaultState"), pipelineState);

	D3D12_RESOURCE_DESC bufferDesc;
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Alignment = 64 KBYTE;
	bufferDesc.Width = sizeof(CameraBufferData);
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT::D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	bufferDesc.Flags = D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_NONE;

	m_constantBuffer = nullptr;
	hresult = m_device->CreatePlacedResource(m_defaultBufferHeap.get(), 0, &bufferDesc, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, nullptr, IID_PPV_ARGS(&m_constantBuffer));

	auto offset = Buffer::calculateAllocSize(sizeof(CameraBufferData));

	bufferDesc.Width = sizeof(Transform) * g_maxMeshInstances;
	m_srvBuffer = nullptr;
	hresult = m_device->CreatePlacedResource(m_defaultBufferHeap.get(), offset, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_srvBuffer));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferDesc;
	cbufferDesc.BufferLocation = m_constantBuffer->GetGPUVirtualAddress();
	cbufferDesc.SizeInBytes = (sizeof(CameraBufferData) + 255) & ~255;
	m_device->CreateConstantBufferView(&cbufferDesc, m_descriptorHeapBuffer->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = g_maxMeshInstances;
	srvDesc.Buffer.StructureByteStride = sizeof(Transform);
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	//m_device->CreateShaderResourceView(m_srvBuffer, &srvDesc, m_descriptorHeapBuffer->GetCPUDescriptorHandleForHeapStart());
	//m_descriptorHeapBuffer->

	return true;
}

RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &desc)
{
	m_taskManager = desc.taskManager;

	m_allocator = vx::StackAllocator(desc.pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);
	const auto doubleBufferSizeInBytes = 5 KBYTE;
	//m_doubleBuffer = DoubleBufferRaw(&m_allocator, doubleBufferSizeInBytes);

	auto result = D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));
	if (result != 0)
	{
		puts("Error creating device");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	result = CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory));
	if (result != 0)
	{
		puts("Error creating dxgi factory");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc;
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue));
	if (result != 0)
	{
		puts("Error creating queue");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_commandAllocator.create(CommandAllocatorType::Direct, m_device))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!m_uploadCmdAllocator.create(CommandAllocatorType::Direct, m_device))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	auto hresult = m_uploadCmdAllocator->Reset();

	m_uploadCommandList = nullptr;
	hresult = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_uploadCmdAllocator.get(), nullptr, IID_PPV_ARGS(&m_uploadCommandList));
	m_uploadCommandList->Close();

	if(!loadShaders())
		return RenderAspectInitializeError::ERROR_SHADER;

	if(!createRootSignature())
		return RenderAspectInitializeError::ERROR_SHADER;

	m_viewport.Height = desc.settings->m_resolution.y;
	m_viewport.Width = desc.settings->m_resolution.x;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.MinDepth = 0.0f;
	m_viewport.TopLeftX = 0;
	m_viewport.TopLeftY = 0;

	m_rectScissor.left = 0;
	m_rectScissor.top = 0;
	m_rectScissor.right = desc.settings->m_resolution.x;
	m_rectScissor.bottom = desc.settings->m_resolution.y;

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
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 1;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = m_device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&m_descriptorHeapRtv));
	if (result != 0)
	{
		puts("Error CreateDescriptorHeap");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	descHeap.NumDescriptors = 16;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	result = m_device->CreateDescriptorHeap(&descHeap, IID_PPV_ARGS(&m_descriptorHeapBuffer));
	if (result != 0)
	{
		puts("Error CreateDescriptorHeap");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	m_swapChain->GetBuffer(0, IID_PPV_ARGS(&m_renderTarget));
	m_device->CreateRenderTargetView(m_renderTarget, nullptr, m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart());

	if (!createHeaps())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createCommandList())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createMeshBuffers())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createPipelineState())
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence));
	m_currentFence = 1;

	m_handleEvent = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);

	waitForGpu();

	return RenderAspectInitializeError::OK;
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
	m_geometryUploadBuffer.release();

	m_uploadHeap.release();

	for (auto &it : m_rootSignatures)
	{
		it->Release();
	}
	m_rootSignatures.clear();

	for (auto &it : m_shaders)
	{
		it->Release();
	}
	m_shaders.clear();

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

	m_uploadCmdAllocator.destroy();
	m_commandAllocator.destroy();

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
	m_taskManager->pushTask(new TaskUpdateCamera(data.position, data.quaternionRotation, &m_camera));

	/*RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::UpdateCamera;

	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_updateCameraData = data;

	m_tasks.push_back(task);*/
}

void RenderAspect::update()
{
	//processTasks();
}

void RenderAspect::updateProfiler(f32 dt)
{

}

void RenderAspect::submitCommands()
{
	auto defaultState = *m_pipelineStates.find(vx::make_sid("defaultState"));
	auto rootSig = *m_rootSignatures.find(vx::make_sid("defaultShader"));

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	vertexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = sizeof(Vertex) * m_vertexCount;
	vertexBufferView.StrideInBytes = sizeof(Vertex);

	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	indexBufferView.BufferLocation = m_indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = sizeof(u32) * m_indexCount;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;

	m_commandAllocator->Reset();

	{
		m_commandList->Reset(m_commandAllocator.get(), nullptr);

		m_commandList->RSSetViewports(1, &m_viewport);
		m_commandList->RSSetScissorRects(1, &m_rectScissor);

		setResourceBarrier(m_commandList, m_renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		m_commandList->ClearRenderTargetView(m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart(), clearColor, 0, nullptr);

		auto tmp = m_descriptorHeapRtv->GetCPUDescriptorHandleForHeapStart();
		m_commandList->OMSetRenderTargets(1, &tmp, true, nullptr);

		setResourceBarrier(m_commandList, m_renderTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

		m_commandList->SetGraphicsRootSignature(rootSig);
		m_commandList->SetPipelineState(defaultState);
		m_commandList->SetDescriptorHeaps(1, &m_descriptorHeapBuffer);
		m_commandList->SetGraphicsRootDescriptorTable(0, m_descriptorHeapBuffer->GetGPUDescriptorHandleForHeapStart());
		m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
		m_commandList->IASetIndexBuffer(&indexBufferView);
		for (auto &it : m_drawCommands)
		{
			m_commandList->DrawIndexedInstanced(it.indexCount, it.instanceCount, it.firstIndex, it.baseVertex, it.baseInstance);
		}

		auto hresult = m_commandList->Close();

		m_cmdLists.push_back(m_commandList);
	}

	auto marker = m_allocator.getMarker();
	u32 count = m_cmdLists.size();
	ID3D12CommandList** ppCommandLists = (ID3D12CommandList**)m_allocator.allocate(sizeof(ID3D12CommandList*) * count, 8);

	for (u32 i = 0;i < count; ++i)
	{
		ppCommandLists[i] = m_cmdLists[i];
	}

	m_commandQueue->ExecuteCommandLists(count, ppCommandLists);

	m_allocator.clear(marker);
	m_cmdLists.clear();
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
		auto scene = (Scene*)evt.arg2.ptr;

		loadScene(scene);

		//TaskLoadScene data;
		//data.ptr = pScene;

		//queueUpdateTask(type, (u8*)&data, sizeof(TaskLoadScene));
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

	std::vector<UploadTaskData> uploadTasks;

	u32 meshIndexOffset = m_meshIndexOffset;

	u32 totalVertexCount = 0;
	u32 totalIndexCount = 0;

	u32 vertexOffsetInBytes = 0;
	u32 indexOffsetInBytes = 0;
	u32 uploadOffsetInBytes = 0;
	for (u32 i = 0; i < meshCount; ++i)
	{
		auto &meshFile = meshes[i];
		auto &mesh = meshFile->getMesh();
		auto sid = meshKeys[i];

		auto vertexCount = mesh.getVertexCount();
		auto vertices = mesh.getVertices();
		auto indexCount = mesh.getIndexCount();
		auto indices = mesh.getIndices();

		totalVertexCount += vertexCount;
		totalIndexCount += indexCount;

		MeshEntry meshEntry;
		meshEntry.indexStart = meshIndexOffset;
		meshEntry.indexCount = indexCount;

		auto vertexSizeInBytes = sizeof(Vertex) * vertexCount;

		UploadTaskData taskUploadVertices;
		taskUploadVertices.src = m_geometryUploadBuffer.get();
		taskUploadVertices.size = vertexSizeInBytes;
		taskUploadVertices.srcOffset = uploadOffsetInBytes;
		taskUploadVertices.dst = m_vertexBuffer;
		taskUploadVertices.dstOffset = vertexOffsetInBytes;
		taskUploadVertices.dstState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;

		MappedBuffer mappedVertexBuffer(uploadOffsetInBytes, vertexSizeInBytes);
		auto error = m_geometryUploadBuffer.map(&mappedVertexBuffer);
		VX_ASSERT(error == 0);

		Vertex* gpuVertices = (Vertex*)mappedVertexBuffer.ptr;
		for (u32 j = 0; j < vertexCount; ++j)
		{
			gpuVertices[j].position = vertices[j].position;
		}
		m_geometryUploadBuffer.unmap(&mappedVertexBuffer);

		vertexOffsetInBytes += vertexSizeInBytes;
		uploadOffsetInBytes += vertexSizeInBytes;

		uploadTasks.push_back(taskUploadVertices);

		auto indexSizeBytes = sizeof(u32) * indexCount;

		UploadTaskData taskUploadIndices;
		taskUploadIndices.src = m_geometryUploadBuffer.get();
		taskUploadIndices.size = indexSizeBytes;
		taskUploadIndices.srcOffset = uploadOffsetInBytes;
		taskUploadIndices.dst = m_indexBuffer;
		taskUploadIndices.dstOffset = indexOffsetInBytes;
		taskUploadIndices.dstState = D3D12_RESOURCE_STATES::D3D12_RESOURCE_STATE_INDEX_BUFFER;

		MappedBuffer mappedIndexBuffer(indexOffsetInBytes, indexSizeBytes);
		error = m_geometryUploadBuffer.map(&mappedIndexBuffer);
		VX_ASSERT(error == 0);
		u32* gpuIndices = (u32*)mappedIndexBuffer.ptr;
		for (u32 j = 0; j < vertexCount; ++j)
		{
			gpuIndices[j] = indices[j];
		}
		m_geometryUploadBuffer.unmap(&mappedIndexBuffer);

		m_meshEntries.insert(sid, meshEntry);

		indexOffsetInBytes += indexSizeBytes;
		uploadOffsetInBytes += indexSizeBytes;

		uploadTasks.push_back(taskUploadIndices);

		meshIndexOffset += indexCount;
	}

	//m_taskManager->pushTask(new );
	TaskUploadGeometry task(&m_uploadCmdAllocator, m_uploadCommandList, std::move(uploadTasks), &m_cmdLists);
	task.run();

	m_meshIndexOffset = meshIndexOffset;

	auto meshInstances = scene->getMeshInstances();
	auto meshInstanceCount = scene->getMeshInstanceCount();

	for (u32 i = 0;i < meshInstanceCount; ++i)
	{
		auto meshSid = meshInstances[i].getMeshSid();

		auto meshIt = m_meshEntries.find(meshSid);
		VX_ASSERT(meshIt != m_meshEntries.end());

		MeshInstanceDrawCmd drawCmd;
		drawCmd.indexCount = meshIt->indexCount;
		drawCmd.instanceCount = 1;
		drawCmd.firstIndex = meshIt->indexStart;
		drawCmd.baseVertex = 0;
		drawCmd.baseInstance = 0;
	}

	m_vertexCount =+ totalVertexCount;
	m_indexCount += totalIndexCount;

	auto lightCount = scene->getLightCount();
	auto lights = scene->getLights();

	/*m_lightRenderer->setLights(lights, lightCount);

	if (m_shadowRenderer)
	{
		m_shadowRenderer->setLights(lights, lightCount);
	}*/
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