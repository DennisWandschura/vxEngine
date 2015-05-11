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
#include "GpuStructs.h"
#include <vxLib/gl/gl.h>
#include <vxLib/gl/Debug.h>
#include <vxLib/gl/StateManager.h>
#include "gl/BufferBindingManager.h"
#include "developer.h"
#include <vxLib/gl/ProgramPipeline.h>
#include "Event.h"
#include "EventTypes.h"
#include "EventsIngame.h"
#include "ScreenshotFactory.h"
#include "DebugRenderSettings.h"
#include <vxLib/Keyboard.h>
#include <vxLib/util/DebugPrint.h>
#include "GpuProfiler.h"
#include "CreateActorData.h"
#include "Locator.h"
#include "EventManager.h"
#include "Graphics/Renderer.h"
#include "Graphics/ShadowRenderer.h"
#include "Graphics/Segment.h"

RenderAspect* g_renderAspect{ nullptr };

namespace
{
	void __stdcall debugCallback()
	{
		std::abort();
	}
}

struct RenderAspect::ColdData
{
	RenderSettings m_settings;
	vx::gl::Texture m_gbufferDepthTexture;
	// albedoSlice : rgb8
	vx::gl::Texture m_gbufferAlbedoSlice;
	// normalSlice : rgb10a2
	vx::gl::Texture m_gbufferNormalSlice;
	// surface : rgba8
	vx::gl::Texture m_gbufferSurfaceSlice;
	// surface : rgbaf16
	vx::gl::Texture m_gbufferTangentSlice;
	vx::gl::Texture m_aabbTexture;
	vx::gl::Buffer m_screenshotBuffer;

	vx::gl::Texture m_ambientColorTexture;
	vx::gl::Texture m_ambientColorBlurTexture[2];
	// contains index into texture array sorted by texture handle

	Font m_font;
	//	std::vector<std::unique_ptr<Graphics::Renderer>> m_renderers;
};

/*class DoubleBufferRaw
{
	u8* m_buffers[2];
	u32 m_currentBuffer;
	u32 m_frontSize;
	u32 m_backSize;
	u32 m_capacity;

	*/
RenderAspect::DoubleBufferRaw::DoubleBufferRaw()
	:m_frontBuffer(nullptr),
	m_backBuffer(nullptr),
	m_frontSize(0),
	m_backSize(0),
	m_capacity(0)
{
}

RenderAspect::DoubleBufferRaw::DoubleBufferRaw(vx::StackAllocator* allocator, u32 capacity)
	:m_frontBuffer(nullptr),
	m_backBuffer(nullptr),
	m_frontSize(0),
	m_backSize(0),
	m_capacity(0)
{
	auto front = allocator->allocate(capacity, 64);
	auto back = allocator->allocate(capacity, 64);

	m_frontBuffer = front;
	m_backBuffer = back;

	m_capacity = capacity;
}

bool RenderAspect::DoubleBufferRaw::memcpy(const u8* data, u32 size)
{
	auto newSize = m_frontSize + size;
	if (newSize >= m_capacity)
		return false;

	::memcpy(m_frontBuffer + m_frontSize, data, size);
	m_frontSize = newSize;

	return true;
}

void RenderAspect::DoubleBufferRaw::swapBuffers()
{
	std::swap(m_frontBuffer, m_backBuffer);
	std::swap(m_frontSize, m_backSize);
	m_frontSize = 0;
}

u8* RenderAspect::DoubleBufferRaw::getBackBuffer()
{
	return m_backBuffer;
}

u32 RenderAspect::DoubleBufferRaw::getBackBufferSize() const
{
	return m_backSize;
}

RenderAspect::RenderAspect()
	:m_shaderManager(),
	m_renderContext(),
	m_camera(),
	m_pColdData()
{
	g_renderAspect = this;
}

RenderAspect::~RenderAspect()
{
}

bool RenderAspect::createBuffers()
{
	m_emptyVao.create();

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Pixel_Pack_Buffer;
		desc.size = m_resolution.x * m_resolution.y * sizeof(f32) * 4;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Read;
		m_pColdData->m_screenshotBuffer.create(desc);
	}

	return true;
}

void RenderAspect::createUniformBuffers()
{
	{
		vx::gl::BufferDescription cameraDesc;
		cameraDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		cameraDesc.size = sizeof(Camerablock);
		cameraDesc.immutable = 1;
		cameraDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
		m_cameraBuffer.create(cameraDesc);
	}

	{
		CamerablockStatic cameraBlockStatic;
		cameraBlockStatic.invProjectionMatrix = vx::MatrixInverse(m_renderContext.getProjectionMatrix());
		cameraBlockStatic.projectionMatrix = m_renderContext.getProjectionMatrix();
		cameraBlockStatic.orthoMatrix = m_renderContext.getOrthoMatrix();

		vx::gl::BufferDescription cameraDesc;
		cameraDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		cameraDesc.immutable = 1;
		cameraDesc.size = sizeof(CamerablockStatic);
		cameraDesc.flags = vx::gl::BufferStorageFlags::None;
		cameraDesc.pData = &cameraBlockStatic;

		m_objectManager.createBuffer("CameraBufferStatic", cameraDesc);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(ShadowTransformBlock);
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;

		m_objectManager.createBuffer("ShadowTransformBuffer", desc);
	}

	{
		UniformTextureBufferBlock data;
		data.u_albedoSlice = m_pColdData->m_gbufferAlbedoSlice.getTextureHandle();
		data.u_normalSlice = m_pColdData->m_gbufferNormalSlice.getTextureHandle();
		data.u_surfaceSlice = m_pColdData->m_gbufferSurfaceSlice.getTextureHandle();
		data.u_tangentSlice = m_pColdData->m_gbufferTangentSlice.getTextureHandle();
		data.u_depthSlice = m_pColdData->m_gbufferDepthTexture.getTextureHandle();
		data.u_aabbTexture = m_pColdData->m_aabbTexture.getTextureHandle();
		data.u_ambientSlice = m_pColdData->m_ambientColorTexture.getTextureHandle();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(UniformTextureBufferBlock);
		desc.immutable = 1;
		desc.pData = &data;

		m_objectManager.createBuffer("UniformTextureBuffer", desc);
	}
}

void RenderAspect::createTextures()
{
	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.size = vx::ushort3(m_resolution.x, m_resolution.y, 1);
		desc.miplevels = 1;
		desc.sparse = 0;

		desc.format = vx::gl::TextureFormat::RGB8;
		m_pColdData->m_gbufferAlbedoSlice.create(desc);
		m_pColdData->m_gbufferAlbedoSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGB16F;
		m_pColdData->m_gbufferNormalSlice.create(desc);
		m_pColdData->m_gbufferNormalSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGBA8;
		m_pColdData->m_gbufferSurfaceSlice.create(desc);
		m_pColdData->m_gbufferSurfaceSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGBA16F;
		m_pColdData->m_gbufferTangentSlice.create(desc);
		m_pColdData->m_gbufferTangentSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::DEPTH32;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.size = vx::ushort3(m_resolution.x, m_resolution.y, 1);
		m_pColdData->m_gbufferDepthTexture.create(desc);
		m_pColdData->m_gbufferDepthTexture.makeTextureResident();
	}

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::RGBA16F;
		desc.size = vx::ushort3(m_resolution.x, m_resolution.y, 1);
		desc.miplevels = 1;
		m_pColdData->m_ambientColorTexture.create(desc);
		m_pColdData->m_ambientColorTexture.makeTextureResident();

		for (int i = 0; i < 2; ++i)
		{
			m_pColdData->m_ambientColorBlurTexture[i].create(desc);
		}
	}

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::RGB32F;
		desc.size = vx::ushort3(1024, 2, 1);
		desc.miplevels = 1;
		m_pColdData->m_aabbTexture.create(desc);

		m_pColdData->m_aabbTexture.setFilter(vx::gl::TextureFilter::NEAREST, vx::gl::TextureFilter::NEAREST);

		m_pColdData->m_aabbTexture.makeTextureResident();
	}
}

void RenderAspect::createFrameBuffers()
{
	{
		m_gbufferFB.create();
		m_gbufferFB.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_gbufferAlbedoSlice, 0);
		m_gbufferFB.attachTexture(vx::gl::Attachment::Color1, m_pColdData->m_gbufferNormalSlice, 0);
		m_gbufferFB.attachTexture(vx::gl::Attachment::Color2, m_pColdData->m_gbufferSurfaceSlice, 0);
		m_gbufferFB.attachTexture(vx::gl::Attachment::Color3, m_pColdData->m_gbufferTangentSlice, 0);
		m_gbufferFB.attachTexture(vx::gl::Attachment::Depth, m_pColdData->m_gbufferDepthTexture, 0);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glNamedFramebufferDrawBuffers(m_gbufferFB.getId(), 4, buffers);
	}

	{
		m_aabbFB.create();
		m_aabbFB.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_aabbTexture, 0);
		glNamedFramebufferDrawBuffer(m_aabbFB.getId(), GL_COLOR_ATTACHMENT0);
	}

	{
		m_coneTraceFB.create();
		m_coneTraceFB.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_ambientColorTexture, 0);
		glNamedFramebufferDrawBuffer(m_coneTraceFB.getId(), GL_COLOR_ATTACHMENT0);
	}

	{
		for (int i = 0; i < 2; ++i)
		{
			m_blurFB[i].create();
			m_blurFB[i].attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_ambientColorBlurTexture[i], 0);
			glNamedFramebufferDrawBuffer(m_blurFB[i].getId(), GL_COLOR_ATTACHMENT0);
		}
	}
}

void RenderAspect::createColdData()
{
	m_pColdData = std::make_unique<ColdData>();
}

bool RenderAspect::initialize(const std::string &dataDir, const RenderAspectDescription &desc)
{
	createColdData();

	vx::gl::ContextDescription contextDesc = vx::gl::ContextDescription::create(*desc.window, desc.resolution, desc.fovRad, desc.z_near, desc.z_far, 4, 5, desc.vsync, desc.debug);
	if (!m_renderContext.initialize(contextDesc))
		return false;

	m_pColdData->m_settings.m_resolution = desc.resolution;
	m_pColdData->m_settings.m_maxActiveLights = 5;
	m_pColdData->m_settings.m_shadowmapResolution = 2048;
	m_pColdData->m_settings.voxelGiQuality = 0;
	m_pColdData->m_settings.m_maxMeshInstances = 150;

	Graphics::Renderer::provide(&m_shaderManager, &m_objectManager, &m_pColdData->m_settings);

	auto result = initializeImpl(dataDir, desc.resolution, desc.debug, desc.pAllocator);

	if (result)
		bindBuffers();

	return result;
}

bool RenderAspect::initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug, vx::StackAllocator *pAllocator)
{
	m_resolution = windowResolution;

	m_allocator = vx::StackAllocator(pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);
	
	if (debug)
	{
		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(::debugCallback);
		vx::gl::Debug::enableCallback(true);
	}

	vx::gl::StateManager::disable(vx::gl::Capabilities::Framebuffer_sRGB);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::setViewport(0, 0, windowResolution.x, windowResolution.y);

	m_objectManager.initialize(50, 20, 20, 10, &m_allocator);

	m_camera.setPosition(0, 2.5f, 15);

	if (!m_shaderManager.initialize(dataDir, true))
	{
		puts("Error initializing Shadermanager");
		return false;
	}

	const auto doubleBufferSizeInBytes = 5 KBYTE;
	m_doubleBuffer = DoubleBufferRaw(&m_allocator, doubleBufferSizeInBytes);

	createTextures();
	createFrameBuffers();

	createUniformBuffers();
	createBuffers();

	m_commandList.initialize();

	m_sceneRenderer.initialize(10, &m_objectManager, pAllocator);
	m_voxelRenderer.initialize(128, m_shaderManager, &m_objectManager);

	{
		auto pShadowRenderer = std::make_unique<Graphics::ShadowRenderer>();
		pShadowRenderer->initialize();

		std::vector<Graphics::Segment> segments;
		pShadowRenderer->getSegments(&segments);
		pShadowRenderer->bindBuffers();

		m_commandList.pushSegment(segments.front(), "segmentCreateShadows", 0);

		m_shadowRenderer = std::move(pShadowRenderer);
		//m_pColdData->m_renderers.push_back(std::move(pShadowRenderer));
	}

	{
		auto file = (dataDir + "textures/verdana.png");
		auto ref = m_sceneRenderer.loadTexture(file.c_str());

		FontAtlas fontAtlas;
		if (!fontAtlas.loadFromFile((dataDir + "fonts/meta/VerdanaRegular.sdff").c_str()))
			return false;

		m_pColdData->m_font = Font(std::move(ref), std::move(fontAtlas));
	}

	m_renderpassFinalImageFullShading.initialize(m_emptyVao, *m_shaderManager.getPipeline("draw_final_image.pipe"), m_resolution);
	m_renderpassFinalImageAlbedo.initialize(m_emptyVao, *m_shaderManager.getPipeline("drawFinalImageAlbedo.pipe"), m_resolution);
	m_renderpassFinalImageNormals.initialize(m_emptyVao, *m_shaderManager.getPipeline("drawFinalImageNormals.pipe"), m_resolution);

	m_pRenderPassFinalImage = &m_renderpassFinalImageFullShading;

	createRenderPassCreateShadowMaps();

	return true;
}

void RenderAspect::createRenderPassCreateShadowMaps()
{

}

bool RenderAspect::initializeProfiler(GpuProfiler* gpuProfiler, vx::StackAllocator* allocator)
{
	auto fontHandle = glGetTextureHandleARB(m_pColdData->m_font.getTextureEntry().getTextureId());

	return m_sceneRenderer.initializeProfiler(m_pColdData->m_font, fontHandle, m_resolution, m_shaderManager, gpuProfiler, allocator);
}

void RenderAspect::shutdown(const HWND hwnd)
{
	m_tasks.clear();
	m_objectManager.shutdown();
	m_pColdData.reset(nullptr);
	m_renderContext.shutdown(hwnd);
}

void RenderAspect::makeCurrent(bool b)
{
	m_renderContext.makeCurrent(b);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTask &task)
{
	std::lock_guard<std::mutex> lck(m_updateMutex);
	m_tasks.push_back(task);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize)
{
	std::lock_guard<std::mutex> lck(m_updateMutex);

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

	std::lock_guard<std::mutex> lck(m_updateMutex);
	m_updateCameraData = data;

	m_tasks.push_back(task);
}

void RenderAspect::update()
{
	std::lock_guard<std::mutex> lck(m_updateMutex);
	m_doubleBuffer.swapBuffers();

	auto backBuffer = m_doubleBuffer.getBackBuffer();
	auto backBufferSize = m_doubleBuffer.getBackBufferSize();
	u32 offset = 0;

	for (auto &it : m_tasks)
	{
		switch (it.type)
		{
		case RenderUpdateTask::Type::UpdateCamera:
			taskUpdateCamera();
			break;
		case RenderUpdateTask::Type::UpdateDynamicTransforms:
			taskUpdateDynamicTransforms(backBuffer + offset, &offset);
			break;
		case RenderUpdateTask::Type::CreateActorGpuIndex:
			taskCreateActorGpuIndex(backBuffer + offset, &offset);
			break;
		case RenderUpdateTask::Type::TakeScreenshot:
			taskTakeScreenshot();
			break;
		case RenderUpdateTask::Type::LoadScene:
			taskLoadScene(backBuffer + offset, &offset);
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

	auto projectionMatrix = m_renderContext.getProjectionMatrix();

	Camerablock block;
	m_camera.getViewMatrix(block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.cameraPosition = m_camera.getPosition();

	m_cameraBuffer.subData(0, sizeof(Camerablock), &block);
}

void RenderAspect::taskTakeScreenshot()
{
	takeScreenshot();
}

void RenderAspect::taskLoadScene(u8* p, u32* offset)
{
	u64 address;
	::memcpy(&address, p, 8);

	Scene* pScene = (Scene*)address;
	vx::verboseChannelPrintF(0, dev::Channel_Render, "Loading Scene into Render");
	m_sceneRenderer.loadScene(*pScene, m_objectManager);
	m_pScene = pScene;

	auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);

	*offset += sizeof(Scene*);
}

void RenderAspect::taskToggleRenderMode()
{
	vx::verboseChannelPrintF(0, dev::Channel_Render, "Toggling Rendermode");

	ShadingMode mode = dev::g_debugRenderSettings.getShadingMode();

	switch (mode)
	{
	case ShadingMode::Full:
		m_pRenderPassFinalImage = &m_renderpassFinalImageFullShading;
		//m_voxelize = dev::g_debugRenderSettings.voxelize();
		break;
	case ShadingMode::Albedo:
		m_pRenderPassFinalImage = &m_renderpassFinalImageAlbedo;
		//m_voxelize = 0;
		break;
	case ShadingMode::Normals:
		m_pRenderPassFinalImage = &m_renderpassFinalImageNormals;
		//m_voxelize = 0;
		break;
	default:
		break;
	}
}

void RenderAspect::taskCreateActorGpuIndex(u8* p, u32* offset)
{
	auto evtManager = Locator::getEventManager();

	CreateActorData* data = (CreateActorData*)p;
	auto gpuIndex = addActorToBuffer(data->transform, data->mesh, data->material, data->pScene);

	Event e;
	e.arg1.u32 = data->index;
	e.arg2.u32 = (u32)gpuIndex;
	e.code = (u32)IngameEvent::Created_Actor_GPU;
	e.type = EventType::Ingame_Event;

	evtManager->addEvent(e);

	*offset += sizeof(CreateActorData);
}

void RenderAspect::taskUpdateDynamicTransforms(u8* ptr, u32* offset)
{
	auto data = (RenderUpdateDataTransforms*)ptr;
	u32 count = data->count;

	auto transforms = (vx::TransformGpu*)(data + 1);
	auto indices = (u32*)(transforms + count);

	for (u32 i = 0; i < count; ++i)
	{
		m_sceneRenderer.updateTransform(transforms[i], indices[i]);
	}

	*offset += sizeof(RenderUpdateDataTransforms) + (sizeof(vx::TransformGpu) + sizeof(u32)) * count;
}

void RenderAspect::render(GpuProfiler* gpuProfiler)
{
	gpuProfiler->pushGpuMarker("clear");
	clearTextures();
	clearBuffers();
	gpuProfiler->popGpuMarker();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

	auto meshCount = m_sceneRenderer.getMeshInstanceCount();
	auto cmdBuffer = m_objectManager.getBuffer("meshCmdBuffer");
	auto meshVao = m_objectManager.getVertexArray("meshVao");
	auto meshParamBuffer = m_objectManager.getBuffer("meshParamBuffer");

	gpuProfiler->pushGpuMarker("shadow mapping");
	m_commandList.draw();
	gpuProfiler->popGpuMarker();

	gpuProfiler->pushGpuMarker("gbuffer");
	createGBuffer(*meshVao, *cmdBuffer, meshCount);
	gpuProfiler->popGpuMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gpuProfiler->pushGpuMarker("voxelize");
	voxelize(*meshVao, *cmdBuffer, meshCount);
	gpuProfiler->popGpuMarker();

	gpuProfiler->pushGpuMarker("mipmap");
	m_voxelRenderer.createMipmaps();
	gpuProfiler->popGpuMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gpuProfiler->pushGpuMarker("cone trace");
	coneTrace();
	gpuProfiler->popGpuMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gpuProfiler->pushGpuMarker("blur");
	blurAmbientColor();
	gpuProfiler->popGpuMarker();

	vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);
	vx::gl::StateManager::bindFrameBuffer(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	gpuProfiler->pushGpuMarker("final");
	m_pRenderPassFinalImage->render(1);
	gpuProfiler->popGpuMarker();

	//voxelDebug();

	renderProfiler(gpuProfiler);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_renderContext.swapBuffers();
}

void RenderAspect::bindBuffers()
{
	auto pShadowTransformBuffer = m_objectManager.getBuffer("ShadowTransformBuffer");
	auto pTextureBuffer = m_objectManager.getBuffer("TextureBuffer");
	auto pUniformTextureBuffer = m_objectManager.getBuffer("UniformTextureBuffer");
	auto pCameraBufferStatic = m_objectManager.getBuffer("CameraBufferStatic");

	gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
	gl::BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(5, pShadowTransformBuffer->getId());

	gl::BufferBindingManager::bindBaseShaderStorage(2, pTextureBuffer->getId());

	m_sceneRenderer.bindBuffers();
	m_voxelRenderer.bindBuffers(m_objectManager);
}

void RenderAspect::clearTextures()
{
	m_voxelRenderer.clearTextures();

	m_pColdData->m_ambientColorTexture.clearImage(0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);

	m_shadowRenderer->clearData();
}

void RenderAspect::clearBuffers()
{
}

void RenderAspect::createGBuffer(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, u32 count)
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer.getId());
	vx::gl::StateManager::bindFrameBuffer(m_gbufferFB);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("create_gbuffer.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	vx::gl::StateManager::bindVertexArray(vao);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));
	//glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 128, sizeof(vx::gl::DrawElementsIndirectCommand));
}

void RenderAspect::voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, u32 count)
{
	m_voxelRenderer.voxelizeScene(count, cmdBuffer, vao);
}

void RenderAspect::voxelDebug()
{
	m_voxelRenderer.debug(m_emptyVao, m_resolution);
}

void RenderAspect::coneTrace()
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);
	glClear(GL_COLOR_BUFFER_BIT);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("coneTrace.pipe");
	auto fs = pPipeline->getFragmentShader();

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::blurAmbientColor()
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());



	auto pPipeline = m_shaderManager.getPipeline("blurpass.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	auto fsShader = pPipeline->getFragmentShader();

	u32 src = 0;
	u32 dst = 1;
	f32 pixelDistance = 2.0f;

//	glProgramUniform1f(fsShader, 0, pixelDistance);

	vx::gl::StateManager::bindFrameBuffer(m_blurFB[dst]);
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	glActiveTexture(GL_TEXTURE0);
	m_pColdData->m_ambientColorTexture.bind();

	glDrawArrays(GL_POINTS, 0, 1);

	std::swap(src, dst);

	for (int i = 2; i < 5; ++i)
	{
		pixelDistance = pow(2.0f, i);
		glProgramUniform1f(fsShader, 0, pixelDistance);

		vx::gl::StateManager::bindFrameBuffer(m_blurFB[dst]);

		glActiveTexture(GL_TEXTURE0);
		m_pColdData->m_ambientColorBlurTexture[src].bind();

		glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
		glDrawArrays(GL_POINTS, 0, 1);

		std::swap(src, dst);
	}

	pPipeline = m_shaderManager.getPipeline("blurpass2.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
	glActiveTexture(GL_TEXTURE0);
	m_pColdData->m_ambientColorBlurTexture[src].bind();

	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);

	glDrawArrays(GL_POINTS, 0, 1);

	//pPipeline = m_shaderManager.getPipeline("blurpass2.pipe");
	//vx::gl::StateManager::bindPipeline(pPipeline->getId());
	//glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::renderProfiler(GpuProfiler* gpuProfiler)
{
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	gpuProfiler->render();

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
}

void RenderAspect::keyPressed(u16 key)
{
	if (key == vx::Keyboard::Key_Num0)
	{
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::ToggleRenderMode;

		dev::g_debugRenderSettings.toggleVoxelize();

		queueUpdateTask(task);
	}
	else if (key == vx::Keyboard::Key_Num7)
	{
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::ToggleRenderMode;

		dev::g_debugRenderSettings.setShadingMode(ShadingMode::Full);

		queueUpdateTask(task);
	}
	else if (key == vx::Keyboard::Key_Num8)
	{
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::ToggleRenderMode;
		dev::g_debugRenderSettings.setShadingMode(ShadingMode::Albedo);
		queueUpdateTask(task);
	}
	else if(key == vx::Keyboard::Key_Num9)
	{
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::ToggleRenderMode;
		dev::g_debugRenderSettings.setShadingMode(ShadingMode::Normals);
		queueUpdateTask(task);
	}
	else if(key == vx::Keyboard::Key_F10)
	{
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::TakeScreenshot;
		queueUpdateTask(task);
	}
}

void RenderAspect::takeScreenshot()
{
	const auto resDim = m_resolution.x * m_resolution.y;
	const auto pixelBufferSizeBytes = sizeof(vx::float4a) * resDim;

	m_pColdData->m_screenshotBuffer.bind();
	glReadPixels(0, 0, m_resolution.x, m_resolution.y, GL_RGBA, GL_FLOAT, 0);

	auto pScreenshotData = (vx::float4a*)_aligned_malloc(pixelBufferSizeBytes, 16);
	auto p = m_pColdData->m_screenshotBuffer.map<u8>(vx::gl::Map::Read_Only);
	memcpy(pScreenshotData, p.get(), pixelBufferSizeBytes);
	p.unmap();

	ScreenshotFactory::writeScreenshotToFile(m_resolution, pScreenshotData);
}

void RenderAspect::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case(EventType::File_Event) :
		handleFileEvent(evt);
		break;
	case(EventType::Ingame_Event) :
		//handleIngameEvent(evt);
		break;
	default:
		break;
	}
}

void RenderAspect::handleFileEvent(const Event &evt)
{
	auto fileEvent = (FileEvent)evt.code;

	switch (fileEvent)
	{
	case FileEvent::Scene_Loaded:
	{
		vx::verboseChannelPrintF(0, dev::Channel_Render, "Queuing loading Scene into Render");
		auto pScene = (Scene*)evt.arg1.ptr;
		
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::LoadScene;

		u64 address = (u64)pScene;
		queueUpdateTask(task, (u8*)&address, sizeof(u64));
	}break;
	default:
		break;
	}
}

void RenderAspect::handleIngameEvent(const Event &evt)
{
	auto type = (IngameEvent)evt.code;

	if (type == IngameEvent::Created_NavGraph)
	{
		//NavGraph* pGraph = (NavGraph*)evt.arg1.ptr;

		//m_navMeshRenderer.updateBuffer(*pGraph);
	}
	else if (type == IngameEvent::Create_Actor)
	{
		auto evtManager = Locator::getEventManager();

		CreateActorData* data = (CreateActorData*)evt.arg2.ptr;
		auto gpuIndex = addActorToBuffer(data->transform, data->mesh, data->material, data->pScene);

		delete(data);

		Event e;
		e.arg1 = evt.arg1;
		e.arg2.u32 = (u32)gpuIndex;
		e.code = (u32)IngameEvent::Created_Actor_GPU;
		e.type = EventType::Ingame_Event;

		evtManager->addEvent(e);
	}
}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	*m = m_renderContext.getProjectionMatrix();
}

u16 RenderAspect::addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material, const Scene* pScene)
{
	auto gpuIndex = m_sceneRenderer.addActorToBuffer(transform, mesh, material, pScene);

	auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);

	return gpuIndex;
}