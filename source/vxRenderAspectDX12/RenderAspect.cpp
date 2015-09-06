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
#include <vxEngineLib/Logfile.h>
#include <vxEngineLib/CreateDynamicMeshData.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/CreateActorData.h>
#include <vxLib/ScopeGuard.h>
#include "GpuLight.h"
#include <vxEngineLib/Light.h>
#include "ResourceView.h"
#include "GBufferRenderer.h"
#include "RenderPassFinal.h"
#include "RenderPassZBuffer.h"
#include "RenderPassZBufferCreateMipmaps.h"
#include "RenderPassAo.h"
#include "RenderPassShading.h"
#include "RenderPassCullLights.h"
#include "RenderPassVisibleLights.h"
#include "RenderPassVoxelize.h"
#include "RenderPassDrawVoxel.h"
#include "RenderPassShadow.h"
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/MeshFile.h>
#include "RenderPassConeTrace.h"

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

RenderSettings RenderAspect::s_settings{};

RenderAspect::RenderAspect()
	:m_device(),
	m_commandList(nullptr),
	m_resourceManager(),
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

void RenderAspect::createRenderPasses()
{
	RenderPass::provideData(&m_shaderManager, &m_resourceManager, &m_uploadManager, &s_settings);

	m_gbufferRenderer = new GBufferRenderer(m_commandAllocator.get(), &m_drawCommandMesh);
	//auto renderPassCullLights = new RenderPassCullLights(m_commandAllocator.get());
	//auto renderPassVisibleLights = new RenderPassVisibleLights(m_commandAllocator.get());

	m_renderPasses.push_back(m_gbufferRenderer);
	//m_renderPasses.push_back(renderPassCullLights);
	//m_renderPasses.push_back(renderPassVisibleLights);
	//m_lightManager.getDrawCommand(0))
	m_renderPasses.push_back(new RenderPassShadow(m_commandAllocator.get(), &m_drawCommandMesh));

	m_renderPasses.push_back(new RenderPassZBuffer(m_commandAllocator.get()));
	m_renderPasses.push_back(new RenderPassZBufferCreateMipmaps(m_commandAllocator.get()));

	m_renderPasses.push_back(new RenderPassVoxelize(m_commandAllocator.get(), &m_drawCommandMesh));

	m_renderPasses.push_back(new RenderPassAO(m_commandAllocator.get()));
	m_renderPasses.push_back(new RenderPassShading(m_commandAllocator.get()));
	m_renderPasses.push_back(new RenderPassConeTrace(m_commandAllocator.get()));
	//m_renderPasses.push_back(new RenderPassDrawVoxel(m_commandAllocator.get()));
	m_renderPasses.push_back(new RenderPassFinal(m_commandAllocator.get(), &m_device));

	//m_lightManager.setRenderPasses(renderPassCullLights, renderPassVisibleLights);
}

void RenderAspect::getRequiredMemory(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, u64* bufferHeapSize, u64* textureHeapSize, u64* rtDsHeapSize)
{
	auto device = m_device.getDevice();
	for (auto &it : m_renderPasses)
	{
		it->getRequiredMemory(bufferHeapSize, textureHeapSize, rtDsHeapSize, device);
	}

	const u32 cbufferSize = 64u KBYTE;

	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances, 64 KBYTE>::size;
	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;
	const auto lightBufferSize = d3d::AlignedSizeType<GpuLight, g_maxLightCount, 64 KBYTE>::size;

	m_materialManager.getRequiredMemory(dimSrgb, dimRgb, bufferHeapSize, textureHeapSize, rtDsHeapSize, device);

	m_drawCommandMesh.getRequiredMemory(g_maxMeshInstances, bufferHeapSize);
	m_lightManager.getRequiredMemory(bufferHeapSize);

	*bufferHeapSize += cbufferSize + transformBufferSize + materialBufferSize + lightBufferSize + transformBufferSize;
}

bool RenderAspect::initializeRenderPasses()
{
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
	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances, 64 KBYTE>::size;
	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;
	const auto lightBufferSize = d3d::AlignedSize<sizeof(GpuLight) * g_maxLightCount + 16, 64 KBYTE>::size;


	const wchar_t* names[] =
	{
		L"constantBuffer",
		L"transformBuffer",
		L"materialBuffer",
		L"lightBuffer",
		L"transformBufferPrev"
	};
	u64 sizes[] = { cbufferSize, transformBufferSize, materialBufferSize, lightBufferSize, transformBufferSize };
	u32 states[] = 
	{
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE
	};

	for (u32 i = 0; i < _countof(names); ++i)
	{
		auto ptr = m_resourceManager.createBuffer(names[i], sizes[i], states[i]);
		if (ptr == nullptr)
			return false;
	}

	if (!m_drawCommandMesh.create(L"drawCmdBuffer", g_maxMeshInstances, &m_resourceManager, m_device.getDevice()))
		return false;

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

	s_settings.m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(fovRad, screenAspect, (f64)desc.settings->m_zNear, (f64)desc.settings->m_zFar);
	s_settings.m_farZ = desc.settings->m_zFar;
	s_settings.m_nearZ = desc.settings->m_zNear;
	s_settings.m_fovRad = fovRad;
	s_settings.m_resolution = desc.settings->m_resolution;
	s_settings.m_gpuLightCount = 64;
	s_settings.m_textureDim = 1024;
	s_settings.m_shadowDim = 2048;

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

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 1;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (!m_device.initialize(*desc.window, cmdQueueDesc, &m_graphicsCommandQueue))
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

	m_currentBuffer = 0;
	m_lastBuffer = 1;

	if (!createCommandList())
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!m_uploadManager.initialize(device))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if(!m_meshManager.initialize(g_maxVertexCount, g_maxIndexCount, g_maxMeshInstances, &m_resourceManager, device, desc.pAllocator))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	u64 bufferHeapSize = 0; 
	u64 textureHeapSize = 0;
	u64 rtDsHeapSize = 0;
	vx::uint3 textureDimSrgb = { 1024, 1024, 32 };
	vx::uint3 textureDimRgb = { 1024, 1024, 64 };
	createRenderPasses();
	getRequiredMemory(textureDimSrgb, textureDimRgb, &bufferHeapSize, &textureHeapSize, &rtDsHeapSize);

	if(!m_resourceManager.initializeHeaps(bufferHeapSize, textureHeapSize, rtDsHeapSize, device))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	if (!m_lightManager.initialize(desc.pAllocator, g_maxLightCount, &m_resourceManager))
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
	createSrvLights(g_maxLightCount);

	for (auto &it : m_renderPasses)
	{
		if (!it->createData(device))
			return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!initializeRenderPasses())
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	m_graphicsCommandQueue.wait();

	return RenderAspectInitializeError::OK;
}

void RenderAspect::shutdown(void* hwnd)
{
	m_graphicsCommandQueue.wait();

	m_drawCommands.clear();
	m_drawCommandMesh.destroy();

	m_shaderManager.shutdown();
	m_msgManager = nullptr;
	m_resourceAspect = nullptr;
	m_taskManager = nullptr;

	m_materialManager.shutdown();

	m_meshManager.shutdown();

	m_uploadManager.shutdown();

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

	for(auto &it : m_renderPasses)
	{
		auto ptr = it;
		delete(ptr);
	}
	m_renderPasses.clear();

	m_resourceManager.shutdown();

	m_commandAllocator.destroy();

	m_graphicsCommandQueue.destroy();
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
	auto projectionMatrixDouble = s_settings.m_projectionMatrix;
	auto resolution = s_settings.m_resolution;

	vx::float4 projInfo;
	projInfo.x = 2.0 / (static_cast<f64>(resolution.x) * projectionMatrixDouble.c[0].m256d_f64[0]);
	projInfo.y = -2.0 / (static_cast<f64>(resolution.y) * projectionMatrixDouble.c[1].m256d_f64[1]);//-2.0 / (resolution.y * projectionMatrixDouble.c[1].m256d_f64[1]);
	projInfo.z = -1.0 / projectionMatrixDouble.c[0].m256d_f64[0];
	projInfo.w = 1.0f / projectionMatrixDouble.c[1].m256d_f64[1];//(1.0 + projectionMatrixDouble.c[1].m256d_f64[2]) / projectionMatrixDouble.c[1].m256d_f64[1];

	m_camera.setPosition(data.position);
	m_camera.setRotation(data.quaternionRotation);

	vx::mat4d viewMatrix;
	m_camera.getViewMatrix(&viewMatrix);

	auto viewMatrixPrev = s_settings.m_viewMatrixPrev;
	auto pvMatrixPrev = projectionMatrixDouble * viewMatrixPrev;
	auto pvMatrix = projectionMatrixDouble * viewMatrix;

	vx::mat4 projMatrix;
	projectionMatrixDouble.asFloat(&projMatrix);

	vx::mat4 vMatrix;
	viewMatrix.asFloat(&vMatrix);

	GpuCameraBufferData bufferData;
	bufferData.position = _mm256_cvtpd_ps(data.position);
	viewMatrix.asFloat(&bufferData.viewMatrix);
	pvMatrix.asFloat(&bufferData.pvMatrix);
	pvMatrixPrev.asFloat(&bufferData.pvMatrixPrev);
	bufferData.invViewMatrix = vx::MatrixInverse(bufferData.viewMatrix);
	bufferData.projMatrix = projMatrix;
	bufferData.invProjMatrix = vx::MatrixInverse(projMatrix);
	bufferData.projInfo = projInfo;
	bufferData.zFar = s_settings.m_farZ;
	bufferData.zNear = s_settings.m_nearZ;

	/*f32 zNear = 0.1;
	f32 zFar = 100.0;

	__m128 p = {-5.4f, 1, -10, 1};
	auto pv = vx::Vector4Transform(vMatrix, p);
	auto pp = vx::Vector3TransformCoord(projMatrix, pv);
	float vs_z = -pv.m128_f32[2] / zFar;

	f32 depth = pp.m128_f32[2];//((zFar + zNear) / (zFar - zNear)) + 1.0f / pp.m128_f32[2] * ((-2.0f  * zFar* zNear) / (zFar - zNear));

	float c0 = zNear * zFar;
	float c1 = zNear - zFar;
	float c2 = zFar;

	float zz = c0 / (depth * c1 + c2);

	float sx = (pp.m128_f32[0] * 0.5f + 0.5f) * 1920.0f;
	float sy = (pp.m128_f32[1] * 0.5f + 0.5f) * 1080.f;

	auto ppp = vx::Vector3TransformCoord(bufferData.invProjMatrix, pp);

	//f32 w = pp.m128_f32[2] * bufferData.invProjMatrix.c[2].m128_f32[3] + bufferData.invProjMatrix.c[3].m128_f32[3];
	//f32 invW = 1.0f / w;
	f32 invWW = vs_z * zFar;

	f32 ppx = (sx / 1920) * 2.0f - 1.0f;
	f32 ppy = 1.0f - (sy / 1080) * 2.0f;
	//1.0 - cc.y * 2.0;
	f32 pvx = ppx * bufferData.invProjMatrix.c[0].m128_f32[0] * invWW;

	f32 projx = (2.0f / 1920.0f) * bufferData.invProjMatrix.c[0].m128_f32[0];
	f32 projz = -1.0f * bufferData.invProjMatrix.c[0].m128_f32[0];
	f32 pvv = (sx * projInfo.x + projz) * invWW;

	f32 pvy = ppy * bufferData.invProjMatrix.c[1].m128_f32[1] * invWW;

	f32 projw = 1.0f * bufferData.invProjMatrix.c[1].m128_f32[1];
	f32 projy = -2.0f / 1080.f * bufferData.invProjMatrix.c[1].m128_f32[1];
	f32 pvvy = (sy * projy + projw) * invWW;*/

	auto constantBuffer = m_resourceManager.getBuffer(L"constantBuffer");

	u32 offset = m_currentBuffer * d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;
	m_uploadManager.pushUploadBuffer((u8*)&bufferData, constantBuffer, 0, sizeof(GpuCameraBufferData), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	s_settings.m_viewMatrixPrev = viewMatrix;

	auto invPvMatrix = vx::MatrixInverse(bufferData.pvMatrix);

	m_frustum.update(invPvMatrix);
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

	auto camPos = m_camera.getPosition();
	auto camRot = m_camera.getRotation();
	__m128 cameraPosition = _mm256_cvtpd_ps(camPos);
	__m128 cameraRotation = _mm256_cvtpd_ps(camRot);

	__m128 cameraDirection = {0, 0, -1, 0};
	cameraDirection = vx::quaternionRotation(cameraDirection, cameraRotation);

	m_lightManager.update(cameraPosition, cameraDirection, m_frustum);
}

void RenderAspect::updateProfiler(f32 dt)
{
}

void RenderAspect::createCbvCamera()
{
	auto constantBuffer = m_resourceManager.getBuffer(L"constantBuffer");
	auto lightBuffer = m_resourceManager.getBuffer(L"lightBuffer");

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbufferViewDesc{};
	cbufferViewDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress();
	cbufferViewDesc.SizeInBytes = d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;

	m_resourceManager.insertConstantBufferView("cameraBufferView", cbufferViewDesc);
}

void RenderAspect::createSrvLights(u32 maxCount)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	srvDesc.Buffer.NumElements = maxCount;
	srvDesc.Buffer.StructureByteStride = sizeof(GpuLight);

	m_resourceManager.insertShaderResourceView("lightBufferView", srvDesc);
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

void RenderAspect::submitCommands()
{
	auto allocMarker = m_allocator.getMarker();
	SCOPE_EXIT
	{
		m_allocator.clear(allocMarker);
	};

	auto renderPassCount = m_renderPasses.size();
	auto commandListCountMax = renderPassCount + 2;
	auto cmdLists = (ID3D12CommandList**)m_allocator.allocate(sizeof(ID3D12CommandList*) * commandListCountMax, 8);

	auto hr = m_commandAllocator->Reset();
	if (hr != 0)
	{
		puts("error");
	}

	u32 index = 2;
	for (u32 i = 0; i < renderPassCount; ++i)
	{
		m_renderPasses[i]->submitCommands(cmdLists, &index);
	}

	auto uploadCmdList = m_uploadManager.update();

	m_commandCopyBuffers->Reset(m_commandAllocator.get(), nullptr);
	copyTransforms(m_commandCopyBuffers);
	m_commandCopyBuffers->Close();

	cmdLists[0] = m_commandCopyBuffers;
	cmdLists[1] = uploadCmdList;

	m_graphicsCommandQueue.execute(index, cmdLists);
}

void RenderAspect::endFrame()
{
	m_device.swapBuffer();
	m_graphicsCommandQueue.wait();

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

D3D12_DRAW_INDEXED_ARGUMENTS RenderAspect::addMeshInstance(const MeshInstance &meshInstance, u32* gpuIndex)
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

	D3D12_DRAW_INDEXED_ARGUMENTS cmdArgs;
	cmdArgs.BaseVertexLocation = cmd.baseVertex;
	cmdArgs.IndexCountPerInstance = cmd.indexCount;
	cmdArgs.InstanceCount = cmd.instanceCount;
	cmdArgs.StartIndexLocation = cmd.firstIndex;
	cmdArgs.StartInstanceLocation = cmd.baseInstance;
	m_drawCommandMesh.uploadDrawCommand(cmd.baseInstance, cmdArgs, &m_uploadManager);

	auto instanceCount = m_meshManager.getInstanceCount();
	m_drawCommandMesh.setCount(instanceCount, &m_uploadManager);

	*gpuIndex = cmd.baseInstance;
	m_meshManager.updateResourceViews(&m_resourceManager);

	return cmdArgs;
}

void RenderAspect::updateLights(const Light* lights, u32 count)
{
	auto gpuLights = std::make_unique<GpuLight[]>(count + 1);
	for (u32 i = 0; i < count; ++i)
	{
		gpuLights[i + 1].position = vx::float4(lights[i].m_position, 1);
		gpuLights[i + 1].falloff = lights[i].m_falloff;
		gpuLights[i + 1].lumen = lights[i].m_lumen;
	} 

	m_lightManager.loadSceneLights(lights, count, m_device.getDevice(), &m_resourceManager, &m_uploadManager);

	auto ptr = (u32*)&gpuLights[0].position.x;
	*ptr= count;

	auto lightBuffer = m_resourceManager.getBuffer(L"lightBuffer");

	auto sizeInBytes = sizeof(GpuLight) * (count + 1);
	m_uploadManager.pushUploadBuffer((u8*)gpuLights.get(), lightBuffer, 0, sizeInBytes, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
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
	auto cmd = addMeshInstance(instance, &gpuIndex);

	auto meshSid = instance.getMeshSid();
	auto transform = instance.getTransform();
	auto bounds = instance.getBounds();

	if (bounds.min.x == FLT_MAX)
	{
		auto mesh = m_resourceAspect->getMesh(meshSid);

		auto vertexCount = mesh->getMesh().getVertexCount();
		auto vertices = mesh->getMesh().getVertices();

		for (u32 i = 0; i < vertexCount; ++i)
		{
			bounds.min = vx::min(bounds.min, vertices[i].position);
			bounds.max = vx::max(bounds.max, vertices[i].position);
		}
	}

	m_lightManager.addStaticMeshInstance(cmd, bounds, &m_uploadManager);

	*offset += sizeof(RenderUpdateTaskAddStaticMeshData);
}

void RenderAspect::taskAddDynamicMeshInstance(const u8* p, u32* offset)
{
	std::size_t* address = (std::size_t*)p;

	CreateDynamicMeshData* data = (CreateDynamicMeshData*)(*address);
	auto &instance = *(data->getMeshInstance());

	u32 gpuIndex = 0;
	addMeshInstance(instance, &gpuIndex);

	data->setGpuIndex(gpuIndex);

	vx::Message e;
	e.code = (u32)IngameMessage::Gpu_AddedDynamicMesh;
	e.type = vx::MessageType::Ingame_Event;
	e.arg1.ptr = data;

	m_msgManager->addMessage(e);

	*offset += sizeof(std::size_t);
}

void RenderAspect::getProjectionMatrix(vx::mat4* m) const
{
	s_settings.m_projectionMatrix.asFloat(m);
}