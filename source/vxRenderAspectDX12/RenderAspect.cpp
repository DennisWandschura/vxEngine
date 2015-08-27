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
#include "ResourceView.h"
#include "GBufferRenderer.h"
#include <Initguid.h>
#include <dxgidebug.h>
#include "RenderPassDrawGBuffer.h"

#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/GpuFunctions.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Material.h>
#include "Vertex.h"
#include <vxEngineLib/Transform.h>
#include "GpuCameraBufferData.h"

const u32 COMPUTE_GUARD_BAND = 192;
const u32 g_cbufferOffsetLightCount = d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;

typedef HRESULT (WINAPI *DXGIGetDebugInterfaceProc)(REFIID riid, void **ppDebug);


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
const u32 g_maxLightCount{64};

RenderAspect::RenderAspect()
	:m_device(),
	m_commandList(nullptr),
	m_resourceManager(),
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

	m_commandCopyBuffers = nullptr;
	result = device->CreateCommandList(1, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.get(), nullptr, IID_PPV_ARGS(&m_commandCopyBuffers));
	if (result != 0)
	{
		puts("error creating command list");
		return false;
	}
	m_commandCopyBuffers->Close();

	return true;
}

void RenderAspect::createRenderPasses(const vx::uint2 &resolution)
{
	m_renderPasses.push_back(new GBufferRenderer(resolution, m_commandAllocator.get()));
	m_renderPasses.push_back(new RenderPassDrawGBuffer(m_commandAllocator.get()));
}

void RenderAspect::getRequiredMemory(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, u64* bufferHeapSize, u64* textureHeapSize, u64* rtDsHeapSize)
{
	auto device = m_device.getDevice();
	for (auto &it : m_renderPasses)
	{
		it->getRequiredMemory(bufferHeapSize, textureHeapSize, rtDsHeapSize, device);
	}

	const u32 cbufferSize = 64u KBYTE;
	const auto drawCmdSize = d3d::AlignedSize<d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size + 256, 64 KBYTE>::size;
	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances, 64 KBYTE>::size;
	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;
	const auto lightBufferSize = d3d::AlignedSizeType<GpuLight, g_maxLightCount, 64 KBYTE>::size;

	m_materialManager.getRequiredMemory(dimSrgb, dimRgb, bufferHeapSize, textureHeapSize, rtDsHeapSize, device);

	*bufferHeapSize += cbufferSize + drawCmdSize + transformBufferSize + materialBufferSize + lightBufferSize + transformBufferSize;
}

bool RenderAspect::initializeRenderPasses()
{
	RenderPass::provideData(&m_shaderManager, &m_resourceManager);
	auto device = m_device.getDevice();

	for (auto &it : m_renderPasses)
	{
		if (!it->initialize(device, nullptr))
			return false;
	}
	return true;
}

bool RenderAspect::createConstantBuffers()
{
	const u32 cbufferSize = 64u KBYTE;
	const auto drawCmdSize = d3d::AlignedSize<d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size + 256, 64 KBYTE>::size;
	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances, 64 KBYTE>::size;
	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;
	const auto lightBufferSize = d3d::AlignedSizeType<GpuLight, g_maxLightCount, 64 KBYTE>::size;


	const wchar_t* names[] =
	{
		L"constantBuffer",
		L"drawCmdBuffer",
		L"transformBuffer",
		L"materialBuffer",
		L"lightBuffer",
		L"transformBufferPrev"
	};
	u64 sizes[6] = { cbufferSize, drawCmdSize, transformBufferSize, materialBufferSize, lightBufferSize, transformBufferSize };
	u32 states[6] = 
	{
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT, 
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
	};

	for (u32 i = 0; i < _countof(names); ++i)
	{
		auto ptr = m_resourceManager.createBuffer(names[i], sizes[i], states[i]);
		if (ptr == nullptr)
			return false;
	}

	return true;
}

RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &desc)
{
	auto errorlog = desc.errorlog;

	m_taskManager = desc.taskManager;
	m_resourceAspect = desc.resourceAspect;
	m_msgManager = desc.msgManager;

	f64 screenAspect = (f64)desc.settings->m_resolution.x / (f64)desc.settings->m_resolution.y;
	f64 fovRad = vx::degToRad(desc.settings->m_fovDeg);
	m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(fovRad, screenAspect, (f64)desc.settings->m_zNear, (f64)desc.settings->m_zFar);
	m_zFar = desc.settings->m_zFar;
	m_zNear = desc.settings->m_zNear;

	const u32 allocSize = 1 MBYTE;
	auto allocPtr = desc.pAllocator->allocate(allocSize);
	m_allocator = vx::StackAllocator(allocPtr, allocSize);

	const auto doubleBufferSizeInBytes = 5 KBYTE;

	auto debugMode = desc.settings->m_renderDebug;
	if (debugMode)
	{
		if (!m_debug.initializeDebugMode())
			return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_device.initialize(*desc.window))
	{
		errorlog->append("error initializing device\n");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}
	auto device = m_device.getDevice();

	if (debugMode)
	{
		if (!m_debug.initialize(desc.pAllocator, device))
			return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	SCOPE_EXIT
	{
		m_debug.printDebugMessages();
	};

	auto hresult = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.getAddressOf()));
	if (hresult != 0)
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}
	m_commandAllocator->SetName(L"CommandAllocatorDefault");

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

	m_resolution = resolution;

	D3D12_DESCRIPTOR_HEAP_DESC descHeap = {};
	descHeap.NumDescriptors = 2;
	descHeap.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descHeap.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (!m_descriptorHeapRtv.create(descHeap, device))
	{
		errorlog->append("Error CreateDescriptorHeap");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	auto rtvHandleCpu = m_descriptorHeapRtv.getHandleCpu();
	auto swapChain = m_device.getSwapChain();
	m_currentBuffer = 0;
	m_lastBuffer = 1;
	for (u32 i = 0; i < 2; ++i)
	{
		swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTarget[i]));

		m_renderTarget[i]->SetName(L"DeviceRenderTarget");

		device->CreateRenderTargetView(m_renderTarget[i], nullptr, rtvHandleCpu);
		rtvHandleCpu.offset(1);
	}

	if (!createCommandList())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!m_uploadManager.initialize(device))
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
	if (hresult != 0)
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if(!m_meshManager.initialize(g_maxVertexCount, g_maxIndexCount, g_maxMeshInstances, &m_resourceManager, device, desc.pAllocator))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	u64 bufferHeapSize = 0; 
	u64 textureHeapSize = 0;
	u64 rtDsHeapSize = 0;
	vx::uint3 textureDimSrgb = { 1024, 1024, 32 };
	vx::uint3 textureDimRgb = { 1024, 1024, 64 };
	createRenderPasses(resolution);
	getRequiredMemory(textureDimSrgb, textureDimRgb, &bufferHeapSize, &textureHeapSize, &rtDsHeapSize);

	if(!m_resourceManager.initializeHeaps(bufferHeapSize, textureHeapSize, rtDsHeapSize, device))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!m_materialManager.initialize(textureDimSrgb, textureDimRgb, &m_allocator, &m_resourceManager, device))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!createConstantBuffers())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	createCbvCamera();
	createSrvTextures(textureDimSrgb.z, textureDimRgb.z);
	createSrvTransformPrev(g_maxMeshInstances);
	createSrvTransform(g_maxMeshInstances);
	createSrvMaterial(g_maxMeshInstances);

	if (!initializeRenderPasses())
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	/*GBufferRendererInitDesc gbufferInitDesc;
	gbufferInitDesc.resolution = resolution;
	if (!m_gbufferRenderer.initialize(&m_shaderManager, &m_resourceManager, device, &gbufferInitDesc))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_drawQuadRenderer.initialize(device, &m_resourceManager, &m_shaderManager))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_renderPassZBuffer.initialize(&m_shaderManager, &m_resourceManager, device, nullptr))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_renderPassZBufferCreateMipmaps.initialize(&m_shaderManager, &m_resourceManager, device, nullptr))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}*/

	return RenderAspectInitializeError::OK;
}

void RenderAspect::shutdown(void* hwnd)
{
	m_device.waitForGpu();

	m_drawCommands.clear();

	m_shaderManager.shutdown();
	m_msgManager = nullptr;
	m_resourceAspect = nullptr;
	m_taskManager = nullptr;

	m_materialManager.shutdown();

	m_meshManager.shutdown();

	m_uploadManager.shutdown();

	m_commandSignature.destroy();

	m_renderTarget[0]->Release();
	m_renderTarget[0] = nullptr;
	m_renderTarget[1]->Release();
	m_renderTarget[1] = nullptr;

	if (m_commandList)
	{
		m_commandList->Release();
		m_commandList = nullptr;
	}

	if (m_commandCopyBuffers)
	{
		m_commandCopyBuffers->Release();
		m_commandCopyBuffers = nullptr;
	}

	while (!m_renderPasses.empty())
	{
		auto ptr = m_renderPasses.back();
		m_renderPasses.pop_back();

		ptr->shutdown();
		delete(ptr);
	}

	m_descriptorHeapRtv.destroy();

	m_resourceManager.shutdown();

	m_commandAllocator.destroy();

	m_debug.shutdownDevice();
	m_device.shutdown();

	m_debug.reportLiveObjects();
	m_debug.shutdown();
}

bool RenderAspect::initializeProfiler(Logfile* errorlog)
{
	return true;
}

void RenderAspect::makeCurrent(bool b)
{
}

void RenderAspect::updateCamera(const RenderUpdateCameraData &data)
{
	/**  vec4(-2.0f / (width*P[0][0]),
	-2.0f / (height*P[1][1]),
	( 1.0f - P[0][2]) / P[0][0],
	( 1.0f + P[1][2]) / P[1][1])

	where P is the projection matrix that maps camera space points
	to [-1, 1] x [-1, 1].  That is, GCamera::getProjectUnit(). */

	vx::mat4 projMatrix;
	m_projectionMatrix.asFloat(&projMatrix);

	vx::float4 projInfo;
	projInfo.x = -2.0f / (m_resolution.x * projMatrix[0].m128_f32[0]);
	projInfo.y = -2.0f / (m_resolution.y * projMatrix[1].m128_f32[1]);
	projInfo.z = (1.0f - projMatrix[0].m128_f32[2]) / projMatrix[0].m128_f32[0];
	projInfo.w = (1.0f + projMatrix[1].m128_f32[2]) / projMatrix[1].m128_f32[1];

	m_camera.setPosition(data.position);
	m_camera.setRotation(data.quaternionRotation);

	vx::mat4d viewMatrix;
	m_camera.getViewMatrix(&viewMatrix);

	auto pvMatrixPrev = m_projectionMatrix * m_viewMatrixPrev;
	auto pvMatrix = m_projectionMatrix * viewMatrix;

	GpuCameraBufferData bufferData;
	bufferData.position = _mm256_cvtpd_ps(m_camera.getPosition());
	viewMatrix.asFloat(&bufferData.viewMatrix);
	pvMatrix.asFloat(&bufferData.pvMatrix);
	pvMatrixPrev.asFloat(&bufferData.pvMatrixPrev);
	bufferData.projMatrix = projMatrix;
	bufferData.projInfo = projInfo;
	bufferData.zFar = m_zFar;
	bufferData.zNear = m_zNear;

	auto constantBuffer = m_resourceManager.getBuffer(L"constantBuffer");

	u32 offset = m_currentBuffer * d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;
	m_uploadManager.pushUploadBuffer((u8*)&bufferData, constantBuffer, 0, sizeof(GpuCameraBufferData), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_viewMatrixPrev = viewMatrix;
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
	m_debug.printDebugMessages();
}

void RenderAspect::updateProfiler(f32 dt)
{
}

void RenderAspect::createCbvCamera()
{
	auto constantBuffer = m_resourceManager.getBuffer(L"constantBuffer");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferViewDesc{};
	cbufferViewDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	cbufferViewDesc.SizeInBytes = d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;

	m_resourceManager.insertConstantBufferView("cameraBufferView", cbufferViewDesc);
}

void RenderAspect::createSrvTransformPrev(u32 instanceCount)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = instanceCount;
	srvDesc.Buffer.StructureByteStride = sizeof(vx::TransformGpu);

	m_resourceManager.insertShaderResourceView("transformBufferPrevView", srvDesc);

}

void RenderAspect::createSrvTransform(u32 instanceCount)
{
	auto transformBuffer = m_resourceManager.getBuffer(L"transformBuffer");

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = instanceCount;
	srvDesc.Buffer.StructureByteStride = sizeof(vx::TransformGpu);

	m_resourceManager.insertShaderResourceView("transformBufferView", srvDesc);
}

void RenderAspect::createSrvMaterial(u32 instanceCount)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = instanceCount;
	srvDesc.Buffer.StructureByteStride = sizeof(u32);

	m_resourceManager.insertShaderResourceView("materialBufferView", srvDesc);
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

	m_resourceManager.insertShaderResourceView("srgbTextureView", srvDesc);

	srvDesc.Format = DXGI_FORMAT_BC7_UNORM;
	srvDesc.Texture2DArray.ArraySize = rgbCount;

	m_resourceManager.insertShaderResourceView("rgbTextureView", srvDesc);
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

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	m_commandList->OMSetRenderTargets(1, rtvHandles, FALSE, nullptr);
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	//m_drawQuadRenderer.submitCommands(m_commandList);

	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTarget[m_currentBuffer], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void RenderAspect::copyTransforms(ID3D12GraphicsCommandList* cmdList)
{
	auto transformBuffer = m_resourceManager.getBuffer(L"transformBuffer");
	auto transformBufferPrev = m_resourceManager.getBuffer(L"transformBufferPrev");

	for (auto &it : m_copyTransforms)
	{
		m_commandCopyBuffers->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(transformBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_SOURCE));
		m_commandCopyBuffers->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(transformBufferPrev, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST));

		u64 offset = sizeof(vx::TransformGpu) * it;
		m_commandCopyBuffers->CopyBufferRegion(transformBufferPrev, offset, transformBuffer, offset, sizeof(vx::TransformGpu));

		m_commandCopyBuffers->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(transformBufferPrev, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
		m_commandCopyBuffers->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(transformBuffer, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE));
	}
	m_copyTransforms.clear();
}

void RenderAspect::renderGBuffer()
{
	//m_commandListDrawMesh->Reset(m_commandAllocator.get(), nullptr);

	//m_renderPasses[0]->submitCommands(m_commandListDrawMesh);

	/*

	if (!m_drawCommands.empty())
	{
		auto drawCmdBuffer = m_resourceManager.getBuffer("LdrawCmdBuffer");

		D3D12_VERTEX_BUFFER_VIEW vertexBufferViews[2];
		vertexBufferViews[0] = m_resourceManager.getResourceView("meshVertexBufferView")->vbv;
		vertexBufferViews[1] = m_resourceManager.getResourceView("meshDrawIdBufferView")->vbv;
		auto indexBufferView = m_resourceManager.getResourceView("meshIndexBufferView")->ibv;

		m_commandListDrawMesh->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_commandListDrawMesh->IASetVertexBuffers(0, 2, vertexBufferViews);
		m_commandListDrawMesh->IASetIndexBuffer(&indexBufferView);

		auto cmdCount = m_drawCommands.size();
		const auto countOffset = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256>::size;
		m_commandListDrawMesh->ExecuteIndirect(m_commandSignature.get(), cmdCount, drawCmdBuffer, 0, drawCmdBuffer, countOffset);
	}*/

	//m_commandListDrawMesh->Close();
}

void RenderAspect::submitCommands()
{
	m_commandAllocator->Reset();
	auto cmdListGBuffer = m_renderPasses[0]->submitCommands();

	m_commandList->Reset(m_commandAllocator.get(), nullptr);

	
//	m_renderPassZBuffer.submitCommands(m_commandList);

	//m_renderPassZBufferCreateMipmaps.submitCommands(m_commandList);

	drawScreenQuadGBuffer(m_commandList);
	auto hresult = m_commandList->Close();
	if (hresult != 0)
	{
		printf("error closing command list\n");
	}

	auto uploadCmdList = m_uploadManager.update();

	m_commandCopyBuffers->Reset(m_commandAllocator.get(), nullptr);
	copyTransforms(m_commandCopyBuffers);
	m_commandCopyBuffers->Close();

	ID3D12CommandList* lists[]
	{
		m_commandCopyBuffers,
		uploadCmdList,
		cmdListGBuffer,
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
	updateTransformStatic(gpuTransform, index);

	auto transformBufferPrev = m_resourceManager.getBuffer(L"transformBufferPrev");
	auto transformOffset = sizeof(vx::TransformGpu) * index;
	m_uploadManager.pushUploadBuffer((u8*)&gpuTransform, transformBufferPrev, transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderAspect::updateTransformStatic(const vx::TransformGpu &transform, u32 index)
{
	auto transformBuffer = m_resourceManager.getBuffer(L"transformBuffer");

	auto transformOffset = sizeof(vx::TransformGpu) * index;
	m_uploadManager.pushUploadBuffer((u8*)&transform, transformBuffer, transformOffset, sizeof(vx::TransformGpu), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
}

void RenderAspect::updateTransformDynamic(const vx::TransformGpu &transform, u32 index)
{
	updateTransformStatic(transform, index);

	copyTransform(index);
}

void RenderAspect::copyTransform(u32 index)
{
	m_copyTransforms.push_back(index);
}

void RenderAspect::addMeshInstance(const MeshInstance &meshInstance, u32* gpuIndex)
{
	auto material = meshInstance.getMaterial();
	u32 materialIndex = 0;
	u32 materialSlices = 0;
	m_materialManager.addMaterial(material, m_resourceAspect, &m_uploadManager, &materialIndex, &materialSlices);

	DrawIndexedCommand cmd;
	m_meshManager.addMeshInstance(meshInstance, materialIndex, m_resourceAspect, &m_resourceManager, &m_uploadManager, &cmd);
	m_drawCommands.push_back(cmd);

	auto meshTransform = meshInstance.getTransform();
	updateTransform(meshTransform, cmd.baseInstance);

	auto materialBuffer = m_resourceManager.getBuffer(L"materialBuffer");
	auto materialOffset = sizeof(u32) * materialIndex;
	m_uploadManager.pushUploadBuffer((u8*)&materialSlices, materialBuffer, materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * cmd.baseInstance;

	auto drawCmdBuffer = m_resourceManager.getBuffer(L"drawCmdBuffer");
	D3D12_DRAW_INDEXED_ARGUMENTS cmdArgs;
	cmdArgs.BaseVertexLocation = cmd.baseVertex;
	cmdArgs.IndexCountPerInstance = cmd.indexCount;
	cmdArgs.InstanceCount = cmd.instanceCount;
	cmdArgs.StartIndexLocation = cmd.firstIndex;
	cmdArgs.StartInstanceLocation = cmd.baseInstance;
	m_uploadManager.pushUploadBuffer((u8*)&cmdArgs, drawCmdBuffer, cmdOffset, sizeof(D3D12_DRAW_INDEXED_ARGUMENTS), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	*gpuIndex = cmd.baseInstance;

	auto instanceCount = m_meshManager.getInstanceCount();

	const auto countOffset = d3d::AlignedSizeType<D3D12_DRAW_INDEXED_ARGUMENTS, g_maxMeshInstances, 256llu>::size;
	u32 count = instanceCount;
	m_uploadManager.pushUploadBuffer((u8*)&count, drawCmdBuffer, countOffset, sizeof(u32), D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT);

	m_meshManager.updateResourceViews(&m_resourceManager);
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

	/*auto lightBuffer = m_resourceManager.getBuffer("lightBuffer");
	m_uploadManager.pushUploadBuffer((u8*)&count, lightBuffer, 0, sizeof(GpuLight) * count, D3D12_RESOURCE_STATE_COMMON);

	auto constantBuffer = m_resourceManager.getBuffer("constantBuffer");
	m_uploadManager.pushUploadBuffer((u8*)&count, constantBuffer, g_cbufferOffsetLightCount, sizeof(u32), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);*/
}

void RenderAspect::loadScene(Scene* scene)
{
	auto lights = scene->getLights();
	auto lightCount = scene->getLightCount();

	updateLights(lights, lightCount);
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
	m_meshManager.addMeshInstance(data->getActorSid(), data->getMeshSid(), materialIndex, m_resourceAspect, &m_resourceManager, &m_uploadManager, &cmd);
	data->setGpu(cmd.baseInstance);

	vx::Message e;
	e.arg1.ptr = data;
	e.code = (u32)IngameMessage::Gpu_AddedActor;
	e.type = vx::MessageType::Ingame_Event;

	m_msgManager->addMessage(e);

	updateTransform(actorTransform, cmd.baseInstance);

	auto materialBuffer = m_resourceManager.getBuffer(L"materialBuffer");
	auto materialOffset = sizeof(u32) * materialIndex;
	m_uploadManager.pushUploadBuffer((u8*)&materialSlices, materialBuffer, materialOffset, sizeof(u32), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	auto cmdOffset = sizeof(D3D12_DRAW_INDEXED_ARGUMENTS) * cmd.baseInstance;

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

		updateTransformDynamic(transform, index);
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

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	m_projectionMatrix.asFloat(m);
}