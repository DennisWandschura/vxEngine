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
#include "RenderPass.h"
#include "RenderLayerGame.h"
#include "GpuCameraBufferData.h"
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib/Logfile.h>
#include <vxLib/ScopeGuard.h>
#include "RenderLayerPerfOverlay.h"
#include <vxEngineLib/ResourceAspectInterface.h>
#include <vxEngineLib/debugPrint.h>
#include <csignal>
#include "FrameData.h"
#include <vxEngineLib/CpuTimer.h>

const u32 g_swapChainBufferCount{ 2 };
const u32 g_maxVertexCount{ 20000 };
const u32 g_maxIndexCount{ 40000 };
const u32 g_maxMeshInstances{ 128 };
const u32 g_maxLightCount{64};
const u8 g_frameCount{ 2 };

RenderSettings RenderAspect::s_settings{};

RenderAspect::RenderAspect()
	:m_graphicsCommandQueue(),
	m_currentFrameData(nullptr),
	m_threadData(nullptr),
	m_fenceEvent(nullptr),
	m_fence(),
	m_currentFenceValue(0),
	m_frameIndex(0),
	m_drawToBackBuffer(),
	m_device(),
	m_activeLayers(),
	m_gpuProfiler(),
	m_copyManager(),
	m_resourceManager(),
	m_camera(),
	m_debug(),
	m_cpuProfiler(nullptr),
	m_frustum(),
	m_downloadManager(),
	m_uploadManager(),
	m_materialManager(),
	m_taskManager(nullptr),
	m_resourceAspect(nullptr),
	m_allocator(),
	m_msgManager(nullptr),
	m_shaderManager()
{

}

RenderAspect::~RenderAspect()
{
}

void RenderAspect::createThreadData(u32 threadCount, vx::StackAllocator* allocator)
{
	auto createFrameData = [](FrameData* frameData, ID3D12Device* device)
	{
		frameData->m_allocatorCopy.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device);
		frameData->m_allocatorDownload.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device);
		frameData->m_allocatorProfiler.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device);
		frameData->m_allocatorUpload.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device);
		frameData->m_commandListProfiler.create(device, D3D12_COMMAND_LIST_TYPE_DIRECT, frameData->m_allocatorProfiler.get());
	};

	auto device = m_device.getDevice();
	m_threadData = (d3d::ThreadData*)allocator->allocate(sizeof(d3d::ThreadData)* threadCount, __alignof(d3d::ThreadData));
	for (u32 i = 0; i < threadCount; ++i)
	{
		allocator->construct(&m_threadData[i].m_bundleAllocator);
		m_threadData[i].m_bundleAllocator.create(D3D12_COMMAND_LIST_TYPE_BUNDLE, device);
		m_threadData[i].m_bundleAllocator->Reset();

		allocator->construct(&m_threadData[i].m_staticAllocator);
		m_threadData[i].m_staticAllocator.create(D3D12_COMMAND_LIST_TYPE_DIRECT, device);
		m_threadData[i].m_staticAllocator->Reset();

		allocator->construct(&m_threadData[i].m_frameData[0]);
		allocator->construct(&m_threadData[i].m_frameData[1]);

		createFrameData(&m_threadData[i].m_frameData[0], device);
		createFrameData(&m_threadData[i].m_frameData[1], device);
	}

	m_currentFrameData = &m_threadData[0].m_frameData[0];
}

void RenderAspect::getRequiredMemory(const vx::uint3 &dimSrgb, const vx::uint3 &dimRgb, u64* bufferHeapSize, u32* bufferCount, u64* textureHeapSize, u32* textureCount, u64* rtDsHeapSize, u32* rtDsCount)
{
	auto device = m_device.getDevice();
	for (auto &it : m_activeLayers)
	{
		it->getRequiredMemory(bufferHeapSize, bufferCount, textureHeapSize, textureCount, rtDsHeapSize, rtDsCount);
	}

	const u32 cbufferSize = 64u KBYTE;

	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances, 64 KBYTE>::size;
	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;

	m_materialManager.getRequiredMemory(dimSrgb, dimRgb, textureHeapSize, textureCount, device);

	m_gpuProfiler.getRequiredMemory(256, bufferHeapSize, bufferCount);

	*bufferHeapSize += cbufferSize + transformBufferSize + materialBufferSize + transformBufferSize;
	*bufferCount += 4;
}

bool RenderAspect::createConstantBuffers()
{
	const u32 cbufferSize = 64u KBYTE;
	const auto transformBufferSize = d3d::AlignedSizeType<vx::TransformGpu, g_maxMeshInstances, 64 KBYTE>::size;
	const auto materialBufferSize = d3d::AlignedSizeType<u32, g_maxMeshInstances, 64 KBYTE>::size;

	const wchar_t* names[] =
	{
		L"constantBuffer",
		L"transformBuffer",
		L"materialBuffer",
		L"transformBufferPrev"
	};
	u64 sizes[] = { cbufferSize, transformBufferSize, materialBufferSize, transformBufferSize };
	u32 states[] = 
	{
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
		D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
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

void RenderAspect::uploadStaticCameraData()
{
	auto projectionMatrixDouble = s_settings.m_projectionMatrix;
	auto resolution = s_settings.m_resolution;

	vx::float4 projInfo;
	projInfo.x = static_cast<f32>(2.0 / (static_cast<f64>(resolution.x) * projectionMatrixDouble.c[0].m256d_f64[0]));
	projInfo.y = static_cast<f32>(-2.0 / (static_cast<f64>(resolution.y) * projectionMatrixDouble.c[1].m256d_f64[1]));//-2.0 / (resolution.y * projectionMatrixDouble.c[1].m256d_f64[1]);
	projInfo.z = static_cast<f32>(-1.0 / projectionMatrixDouble.c[0].m256d_f64[0]);
	projInfo.w = static_cast<f32>(1.0f / projectionMatrixDouble.c[1].m256d_f64[1]);//(1.0 + projectionMatrixDouble.c[1].m256d_f64[2]) / projectionMatrixDouble.c[1].m256d_f64[1];

	vx::mat4 projMatrix;
	projectionMatrixDouble.asFloat(&projMatrix);

	GpuCameraStatic cameraStaticData;
	cameraStaticData.orthoMatrix = vx::MatrixOrthographicRHDX(static_cast<f32>(resolution.x), static_cast<f32>(resolution.y), static_cast<f32>(s_settings.m_nearZ), static_cast<f32>(s_settings.m_farZ));
	cameraStaticData.invProjMatrix = vx::MatrixInverse(projMatrix);
	cameraStaticData.projInfo = projInfo;
	cameraStaticData.zFar = static_cast<f32>(s_settings.m_farZ);
	cameraStaticData.zNear = static_cast<f32>(s_settings.m_nearZ);
	cameraStaticData.resolution = vx::float2a(resolution);

	auto constantBuffer = m_resourceManager.getBuffer(L"constantBuffer");
	auto offset = d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;
	m_uploadManager.pushUploadBuffer((u8*)&cameraStaticData, constantBuffer->get(), offset, sizeof(GpuCameraStatic), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
}

bool RenderAspect::setSignalHandler(AbortSignalHandlerFun signalHandlerFn)
{
	auto previousHandler = std::signal(SIGABRT, signalHandlerFn);
	if (previousHandler == SIG_ERR)
	{
		return false;
	}

	return true;
}

RenderAspectInitializeError RenderAspect::initializeImpl(const RenderAspectDescription &desc)
{
	vx::activateChannel(vx::debugPrint::Channel_Render);

	auto errorlog = desc.errorlog;
	m_cpuProfiler = desc.cpuProfiler;

	Event::setAllocator(desc.smallObjAllocatorMainThread);

	m_taskManager = desc.taskManager;
	m_resourceAspect = desc.resourceAspect;
	m_msgManager = desc.msgManager;

	auto resolution = desc.settings->m_resolution;
	f64 screenAspect = (f64)resolution.x / (f64)resolution.y;
	f64 fovRad = vx::degToRad(desc.settings->m_fovDeg);

	s_settings.m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(fovRad, screenAspect, (f64)desc.settings->m_zNear, (f64)desc.settings->m_zFar);
	s_settings.m_farZ = desc.settings->m_zFar;
	s_settings.m_nearZ = desc.settings->m_zNear;
	s_settings.m_fovRad = fovRad;
	s_settings.m_resolution = resolution;
	s_settings.m_gpuLightCount = 64;
	s_settings.m_shadowCastingLightCount = 10;
	s_settings.m_textureDim = 1024;
	s_settings.m_shadowDim = 512;
	s_settings.m_lpvDim = 32;
	s_settings.m_lpvGridSize = 36.f;
	s_settings.m_lpvMip = 3;

	const u32 allocSize = 1 MBYTE;
	auto allocPtr = desc.pAllocator->allocate(allocSize);
	m_allocator = vx::StackAllocator(allocPtr, allocSize);

	const auto doubleBufferSizeInBytes = 5 KBYTE;

	auto debugMode = desc.settings->m_renderDebug;
	if (debugMode)
	{
		if (!m_debug.getDebugInterface())
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error getting debug interface, disabling it");
			debugMode = false;
			//return RenderAspectInitializeError::ERROR_CONTEXT;
		}
	}

	wchar_t shaderRootDir[20];
#if _DEBUG
	swprintf(shaderRootDir, 20, L"../../lib/Debug/");
#else
	swprintf(shaderRootDir, 20, L"../../lib/Release/");
#endif
	m_shaderManager.initialize(shaderRootDir);

	const u32 backBufferCount = 2;

	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc;
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	cmdQueueDesc.NodeMask = 1;
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	cmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	if (!m_device.initialize(*desc.window, cmdQueueDesc, 64, &m_graphicsCommandQueue, backBufferCount))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing device");
		errorlog->append("error initializing device\n");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}
	m_device->SetStablePowerState(1);

	auto device = m_device.getDevice();

	if (debugMode)
	{
		if (!m_debug.initialize(desc.pAllocator, device, errorlog))
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing debug");
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}
	}

	u32 threadCount = 1;
	createThreadData(threadCount, desc.pAllocator);

	SCOPE_EXIT
	{
		if (debugMode)
		{
			m_debug.printDebugMessages();
		}
	};

	m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);

	if (m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.getAddressOf())) != 0)
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_uploadManager.initialize(device, m_threadData[0].m_frameData, g_frameCount))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing upload manager");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_copyManager.initialize(device, m_threadData[0].m_frameData, g_frameCount))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing copy manager");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_downloadManager.initialize(device, m_threadData[0].m_frameData, g_frameCount))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing download manager");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	u64 bufferHeapSize = 0; 
	u64 textureHeapSize = 0;
	u64 rtDsHeapSize = 0;
	vx::uint3 textureDimSrgb = { 1024, 1024, 32 };
	vx::uint3 textureDimRgb = { 1024, 1024, 64 };

	RenderLayerGameDesc renderLayerGameDesc;
	renderLayerGameDesc.m_camera = &m_camera;
	renderLayerGameDesc.m_copyManager = &m_copyManager;
	renderLayerGameDesc.m_device = &m_device;
	renderLayerGameDesc.m_frustum = &m_frustum;
	renderLayerGameDesc.m_materialManager = &m_materialManager;
	renderLayerGameDesc.m_msgManager = m_msgManager;
	renderLayerGameDesc.m_resourceAspect = m_resourceAspect;
	renderLayerGameDesc.m_resourceManager = &m_resourceManager;
	renderLayerGameDesc.m_uploadManager = &m_uploadManager;
	renderLayerGameDesc.m_downloadManager = &m_downloadManager;
	renderLayerGameDesc.m_settings = &s_settings;
	renderLayerGameDesc.m_cpuProfiler = m_cpuProfiler;
	renderLayerGameDesc.m_renderAspect = this;
	auto renderLayerGame = new RenderLayerGame(renderLayerGameDesc);
	m_activeLayers.push_back(renderLayerGame);

	m_activeLayers.push_back(new RenderLayerPerfOverlay(&m_device, m_resourceAspect, &m_uploadManager, &m_resourceManager, resolution));

	RenderPass::provideData(&m_shaderManager, &m_resourceManager, &m_uploadManager, &s_settings, &m_gpuProfiler, m_threadData);

	for (auto &it : m_activeLayers)
	{
		it->createRenderPasses();
	}

	u32 bufferCount = 0, textureCount = 0, rtDsCount = 0;
	getRequiredMemory(textureDimSrgb, textureDimRgb, &bufferHeapSize,&bufferCount, &textureHeapSize, &textureCount, &rtDsHeapSize, &rtDsCount);

	if (!m_resourceManager.initializeHeaps(bufferHeapSize, bufferCount, textureHeapSize, textureCount, rtDsHeapSize, rtDsCount, device, errorlog))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error creating gpu heaps");
		errorlog->append("Error creating gpu heaps\n");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!m_materialManager.initialize(textureDimSrgb, textureDimRgb, &m_allocator, &m_resourceManager, device))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing material manager");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	if (!createConstantBuffers())
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error creating constant buffers");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	auto halResolution = (resolution / vx::uint2(2));
	vx::float2 gpuProfilerPosition = vx::float2(-static_cast<f32>(halResolution.x), static_cast<f32>(halResolution.y));
	gpuProfilerPosition += vx::float2(10, -20);
	if (!m_gpuProfiler.initialize(256, &m_resourceManager, device, m_graphicsCommandQueue.get(), gpuProfilerPosition))
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing gpu profiler");
		errorlog->append("error initializing gpu profiler\n");
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	createCbvCamera();
	createSrvTextures(textureDimSrgb.z, textureDimRgb.z);
	createSrvTransformPrev(g_maxMeshInstances);
	createSrvTransform(g_maxMeshInstances);
	createSrvMaterial(g_maxMeshInstances);

	for (auto &it : m_activeLayers)
	{
		if (!it->createData())
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error creating render layer data");
			errorlog->append("error creating render layer data\n");
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}
	}

	for (auto &it : m_activeLayers)
	{
		if (!it->initialize(g_frameCount, &m_allocator, errorlog))
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "error initializing render layers");
			errorlog->append("error initializing render layers\n");
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}
	}

	if (!m_drawToBackBuffer.initialize(&m_device, &m_shaderManager, &m_resourceManager, backBufferCount, g_frameCount))
	{
		return RenderAspectInitializeError::ERROR_CONTEXT;
	}

	uploadStaticCameraData();

	m_frameIndex = 0;//m_device.getCurrentBackBufferIndex();
	waitForGpu();

	return RenderAspectInitializeError::OK;
}

void RenderAspect::waitForGpu()
{
	auto lastFenceValue = m_currentFenceValue;

	m_graphicsCommandQueue.signal(m_fence.get(), m_currentFenceValue);
	++m_currentFenceValue;

	auto lastCompletedFence = m_fence->GetCompletedValue();

	if (lastCompletedFence < lastFenceValue)
	{
		auto event = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		m_fence->SetEventOnCompletion(lastFenceValue, event);
		WaitForSingleObject(event, INFINITE);
	}
}

void RenderAspect::shutdown(void* hwnd)
{
	wait();

	m_shaderManager.shutdown();
	m_msgManager = nullptr;
	m_resourceAspect = nullptr;
	m_taskManager = nullptr;

	m_materialManager.shutdown();

	m_downloadManager.shutdown();
	m_uploadManager.shutdown();

	m_resourceManager.shutdown();

	m_graphicsCommandQueue.destroy();
	m_debug.shutdownDevice();
	m_device.shutdown();

	//m_debug.reportLiveObjects();
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

	m_camera.setPosition(data.position);
	m_camera.setRotation(data.quaternionRotation);

	vx::mat4d viewMatrix;
	m_camera.getViewMatrixRH(&viewMatrix);

	auto viewMatrixPrev = s_settings.m_viewMatrixPrev;
	auto pvMatrixPrev = projectionMatrixDouble * viewMatrixPrev;
	auto pvMatrix = projectionMatrixDouble * viewMatrix;

	vx::mat4 projMatrix;
	projectionMatrixDouble.asFloat(&projMatrix);

	vx::mat4 vMatrix;
	viewMatrix.asFloat(&vMatrix);

	GpuCameraBufferData bufferData;
	viewMatrix.asFloat(&bufferData.viewMatrix);
	pvMatrix.asFloat(&bufferData.pvMatrix);
	pvMatrixPrev.asFloat(&bufferData.pvMatrixPrev);
	bufferData.invViewMatrix = vx::MatrixInverse(bufferData.viewMatrix);
	bufferData.position = _mm256_cvtpd_ps(data.position);

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

	m_uploadManager.pushUploadBuffer((u8*)&bufferData, constantBuffer->get(), 0, sizeof(GpuCameraBufferData), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	s_settings.m_viewMatrixPrev = viewMatrix;

	auto invPvMatrix = vx::MatrixInverse(bufferData.pvMatrix);

	m_frustum.update(invPvMatrix);
}

void RenderAspect::queueUpdate(const RenderUpdateTaskType type, const u8* data, u32 dataSize)
{
	for (auto &it : m_activeLayers)
	{
		it->queueUpdate(type, data, dataSize);
	}
	/*switch (type)
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
	}*/

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
	for (auto &it : m_activeLayers)
	{
		it->update();
	}

	m_debug.printDebugMessages();

	m_gpuProfiler.update(this);

	/*auto camPos = m_camera.getPosition();
	auto camRot = m_camera.getRotation();
	__m128 cameraPosition = _mm256_cvtpd_ps(camPos);
	__m128 cameraRotation = _mm256_cvtpd_ps(camRot);

	__m128 cameraDirection = {0, 0, -1, 0};
	cameraDirection = vx::quaternionRotation(cameraDirection, cameraRotation);

	m_lightManager.update(cameraPosition, cameraDirection, m_frustum);*/
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

	cbufferViewDesc.BufferLocation = constantBuffer->GetGPUVirtualAddress() + d3d::AlignedSizeType<GpuCameraBufferData, 1, 256>::size;
	cbufferViewDesc.SizeInBytes = d3d::AlignedSizeType<GpuCameraStatic, 1, 256>::size;
	m_resourceManager.insertConstantBufferView("cameraStaticBufferView", cbufferViewDesc);
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

void RenderAspect::buildCommands()
{
	m_downloadManager.downloadToCpu();

	m_gpuProfiler.frame(&m_resourceManager, m_currentFrameData);

	m_copyManager.buildCommandList(m_currentFrameData, m_frameIndex);
	m_downloadManager.buildCommandList(m_currentFrameData, m_frameIndex);
	m_uploadManager.buildCommandList(m_currentFrameData, m_frameIndex);

	for (auto &it : m_activeLayers)
	{
		it->buildCommandLists(m_frameIndex);
	}
}

void RenderAspect::submitCommands()
{
	m_gpuProfiler.submitCommandList(&m_graphicsCommandQueue);

	m_copyManager.submitList(&m_graphicsCommandQueue);
	m_downloadManager.submitCommandList(&m_graphicsCommandQueue);
	m_uploadManager.submitCommandList(&m_graphicsCommandQueue);
	m_graphicsCommandQueue.execute();

	for (auto &it : m_activeLayers)
	{
		it->submitCommandLists(&m_graphicsCommandQueue);
	}

	m_drawToBackBuffer.submitList(&m_graphicsCommandQueue, &m_device, m_frameIndex);
	m_graphicsCommandQueue.execute();

	m_currentFrameData->m_fenceValue = m_currentFenceValue;
	m_graphicsCommandQueue.signal(m_fence.get(), m_currentFenceValue);
	++m_currentFenceValue;
}

void RenderAspect::swapBuffers()
{
	m_device.present();
}

void RenderAspect::wait()
{
	m_frameIndex = (m_frameIndex + 1) & 1;
	m_currentFrameData = &m_threadData[0].m_frameData[m_frameIndex];
	const UINT64 lastCompletedFence = m_fence->GetCompletedValue();
	// Make sure that this frame resource isn't still in use by the GPU.
	// If it is, wait for it to complete.

	if (m_currentFrameData->m_fenceValue > lastCompletedFence)
	{
		m_fence->SetEventOnCompletion(m_currentFrameData->m_fenceValue, m_fenceEvent);
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}
}

void RenderAspect::handleMessage(const vx::Message &msg)
{
	for (auto &it : m_activeLayers)
	{
		it->handleMessage(msg);
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

void RenderAspect::getProjectionMatrix(vx::mat4* m) const
{
	s_settings.m_projectionMatrix.asFloat(m);
}