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
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
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
#include <vxLib/Window.h>
#include "CpuProfiler.h"
#include "Graphics/CommandListFactory.h"

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
	VoxelRenderer m_voxelRenderer;
	vx::gl::Texture m_gbufferDepthTexture;
	// albedoSlice : rgb8
	vx::gl::Texture m_gbufferAlbedoSlice;
	// normalSlice : rgb10a2
	vx::gl::Texture m_gbufferNormalSlice;
	// surface : rgba8
	vx::gl::Texture m_gbufferSurfaceSlice;
	// surface : rgbaf16
	vx::gl::Texture m_gbufferTangentSlice;
	vx::gl::Texture m_gbufferBitangentSlice;
	vx::gl::Texture m_aabbTexture;
	vx::gl::Buffer m_screenshotBuffer;

	vx::gl::Texture m_ambientColorTexture;
	vx::gl::Texture m_ambientColorBlurTexture[2];
	// contains index into texture array sorted by texture handle

	Font m_font;
};

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
	m_objectManager.createVertexArray("emptyVao");

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Pixel_Pack_Buffer;
		desc.size = m_resolution.x * m_resolution.y * sizeof(f32) * 4;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Read;
		m_pColdData->m_screenshotBuffer.create(desc);
	}

	{

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
		desc.size = sizeof(u32) * m_resolution.x * m_resolution.y / 4;
		desc.immutable = 1;

		m_objectManager.createBuffer("ShaderStorageConetracePixelListBuffer", desc);
	}

	{
		vx::gl::DrawArraysIndirectCommand cmd;
		cmd.baseInstance = 0;
		cmd.count = 0;
		cmd.first = 0;
		cmd.instanceCount = 1;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);
		desc.immutable = 1;
		desc.pData = &cmd;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;

		m_objectManager.createBuffer("ShaderStoragePixelListCmdBuffer", desc);
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
		data.u_bitangentSlice = m_pColdData->m_gbufferBitangentSlice.getTextureHandle();
		data.u_depthSlice = m_pColdData->m_gbufferDepthTexture.getTextureHandle();
		data.u_aabbTexture = m_pColdData->m_aabbTexture.getTextureHandle();
		data.u_ambientSlice = m_pColdData->m_ambientColorTexture.getTextureHandle();
		data.u_ambientImage = m_pColdData->m_ambientColorTexture.getImageHandle(0, 0, 0);

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(UniformTextureBufferBlock);
		desc.immutable = 1;
		desc.pData = &data;

		m_objectManager.createBuffer("UniformTextureBuffer", desc);
	}

	{
		RenderSettingsBlock data;
		data.resolution = m_resolution;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(RenderSettingsBlock);
		desc.immutable = 1;
		desc.pData = &data;

		m_objectManager.createBuffer("RenderSettingsBufferBlock", desc);
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

		desc.format = vx::gl::TextureFormat::RGBA16F;
		m_pColdData->m_gbufferNormalSlice.create(desc);
		m_pColdData->m_gbufferNormalSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGBA8;
		m_pColdData->m_gbufferSurfaceSlice.create(desc);
		m_pColdData->m_gbufferSurfaceSlice.makeTextureResident();

		desc.format = vx::gl::TextureFormat::RGBA16F;
		m_pColdData->m_gbufferTangentSlice.create(desc);
		m_pColdData->m_gbufferTangentSlice.makeTextureResident();

		m_pColdData->m_gbufferBitangentSlice.create(desc);
		m_pColdData->m_gbufferBitangentSlice.makeTextureResident();

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

		glMakeImageHandleResidentARB(m_pColdData->m_ambientColorTexture.getImageHandle(0, 0, 0), GL_WRITE_ONLY);

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
		m_gbufferFB.attachTexture(vx::gl::Attachment::Color4, m_pColdData->m_gbufferBitangentSlice, 0);
		m_gbufferFB.attachTexture(vx::gl::Attachment::Depth, m_pColdData->m_gbufferDepthTexture, 0);

		GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4 };
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
	m_pColdData = vx::make_unique<ColdData>();
}

void RenderAspect::provideRenderData(const EngineConfig* settings)
{
	Graphics::Renderer::provide(&m_shaderManager, &m_objectManager, settings);
}

bool RenderAspect::initialize(const std::string &dataDir, const RenderAspectDescription &desc, const EngineConfig* settings)
{
	vx::gl::OpenGLDescription glDescription;
	glDescription.bDebugMode = desc.debug;
	glDescription.bVsync = desc.vsync;
	glDescription.farZ = desc.z_far;
	glDescription.fovRad = desc.fovRad;
	glDescription.hwnd = desc.window->getHwnd();
	glDescription.majVersion = 4;
	glDescription.minVersion = 5;
	glDescription.nearZ = desc.z_near;
	glDescription.resolution = desc.resolution;

	vx::gl::ContextDescription contextDesc;
	contextDesc.glParams = glDescription;
	contextDesc.hInstance = desc.window->getHinstance();
	contextDesc.windowClass = desc.window->getClassName();

	if (!initializeCommon(contextDesc, settings))
		return false;

	auto result = initializeImpl(dataDir, desc.resolution, desc.debug, desc.pAllocator);

	if (result)
		bindBuffers();

	return result;
}

bool RenderAspect::initializeCommon(const vx::gl::ContextDescription &contextDesc, const EngineConfig* settings)
{
	createColdData();

	if (!m_renderContext.initialize(contextDesc))
		return false;

	provideRenderData(settings);

	return true;
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
	m_pColdData->m_voxelRenderer.initialize(128, m_shaderManager, &m_objectManager);

	{

		auto pShadowRenderer = vx::make_unique<Graphics::ShadowRenderer>();
		pShadowRenderer->initialize();

		std::vector<std::pair<std::string,Graphics::Segment>> segments;
		pShadowRenderer->getSegments(&segments);
		pShadowRenderer->bindBuffers();

		for (auto &it : segments)
		{
			m_commandList.pushSegment(it.second, it.first.c_str());
		}

		m_shadowRenderer = std::move(pShadowRenderer);
		//m_pColdData->m_renderers.push_back(std::move(pShadowRenderer));
	}

	//Graphics::CommandListFactory::createFromFile("commandList.txt", m_objectManager, m_shaderManager, &m_commandList);

	{
		auto file = (dataDir + "textures/verdana.png");
		auto ref = m_sceneRenderer.loadTexture(file.c_str());

		FontAtlas fontAtlas;
		if (!fontAtlas.loadFromFile((dataDir + "fonts/meta/VerdanaRegular.sdff").c_str()))
			return false;

		m_pColdData->m_font = Font(std::move(ref), std::move(fontAtlas));
	}

	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	m_renderpassFinalImageFullShading.initialize(*emptyVao, *m_shaderManager.getPipeline("draw_final_image.pipe"), m_resolution);
	m_renderpassFinalImageAlbedo.initialize(*emptyVao, *m_shaderManager.getPipeline("drawFinalImageAlbedo.pipe"), m_resolution);
	m_renderpassFinalImageNormals.initialize(*emptyVao, *m_shaderManager.getPipeline("drawFinalImageNormals.pipe"), m_resolution);

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
		auto p = backBuffer + offset;

		switch (it.type)
		{
		case RenderUpdateTask::Type::UpdateCamera:
			taskUpdateCamera();
			break;
		case RenderUpdateTask::Type::UpdateDynamicTransforms:
			taskUpdateDynamicTransforms(p, &offset);
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

	auto projectionMatrix = m_renderContext.getProjectionMatrix();

	Camerablock block;
	m_camera.getViewMatrix(&block.viewMatrix);
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

	m_shadowRenderer->updateDrawCmds();

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

	vx::Event e;
	e.arg1.u32 = data->index;
	e.arg2.u32 = (u32)gpuIndex;
	e.code = (u32)IngameEvent::Created_Actor_GPU;
	e.type = vx::EventType::Ingame_Event;

	evtManager->addEvent(e);

	*offset += sizeof(CreateActorData);
}

void RenderAspect::taskUpdateDynamicTransforms(u8* p, u32* offset)
{
	auto data = (RenderUpdateDataTransforms*)p;
	u32 count = data->count;

	auto transforms = (vx::TransformGpu*)(data + 1);
	auto indices = (u32*)(p + sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count);

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

	auto instanceCount = m_sceneRenderer.getMeshInstanceCount();
	auto meshCmdBuffer = m_objectManager.getBuffer("meshCmdBuffer");
	auto meshVao = m_objectManager.getVertexArray("meshVao");
	auto meshParamBuffer = m_objectManager.getBuffer("meshParamBuffer");

	gpuProfiler->pushGpuMarker("shadow mapping");
	m_commandList.draw();
	gpuProfiler->popGpuMarker();

	gpuProfiler->pushGpuMarker("gbuffer");
	{
		vx::gl::StateManager::setClearColor(0, 0, 0, 0);
		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindFrameBuffer(m_gbufferFB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, *meshParamBuffer);

		auto pPipeline = m_shaderManager.getPipeline("drawMeshToGBuffer.pipe");
		vx::gl::StateManager::bindPipeline(pPipeline->getId());
		vx::gl::StateManager::bindVertexArray(*meshVao);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, *meshCmdBuffer);
		//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, instanceCount, sizeof(vx::gl::DrawElementsIndirectCommand));
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));
	}
	gpuProfiler->popGpuMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gpuProfiler->pushGpuMarker("voxelize");
	{
		auto voxelFB = m_objectManager.getFramebuffer("voxelFB");

		auto pipeline = m_shaderManager.getPipeline("voxelize.pipe");
		auto geomShader = pipeline->getGeometryShader();

		vx::gl::StateManager::setClearColor(0, 0, 0, 0);
		vx::gl::StateManager::bindFrameBuffer(*voxelFB);

		glClear(GL_COLOR_BUFFER_BIT);

		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);

		vx::gl::StateManager::bindPipeline(*pipeline);
		vx::gl::StateManager::bindVertexArray(*meshVao);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, *meshCmdBuffer);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, *meshParamBuffer);

		glProgramUniform1i(geomShader, 0, 0);
		vx::gl::StateManager::setViewport(0, 0, 128, 128);
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));

		glProgramUniform1i(geomShader, 0, 1);
		vx::gl::StateManager::setViewport(0, 0, 64, 64);
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));

		glProgramUniform1i(geomShader, 0, 2);
		vx::gl::StateManager::setViewport(0, 0, 32, 32);
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));

		glProgramUniform1i(geomShader, 0, 3);
		vx::gl::StateManager::setViewport(0, 0, 16, 16);
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));

		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);

		vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
	}
	gpuProfiler->popGpuMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gpuProfiler->pushGpuMarker("pixel list");
	createConeTracePixelList();
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
	auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");
	auto shaderStorageConetracePixelListBuffer = m_objectManager.getBuffer("ShaderStorageConetracePixelListBuffer");
	auto renderSettingsBufferBlock = m_objectManager.getBuffer("RenderSettingsBufferBlock");
	auto pVoxelBuffer = m_objectManager.getBuffer("VoxelBuffer");
	auto pVoxelTextureBuffer = m_objectManager.getBuffer("VoxelTextureBuffer");
	auto transformBuffer = m_objectManager.getBuffer("transformBuffer");
	auto materialBlockBuffer = m_objectManager.getBuffer("materialBlockBuffer");
	auto lightDataBuffer = m_objectManager.getBuffer("lightDataBuffer");
	auto lightCmdBuffer = m_objectManager.getBuffer("lightCmdBuffer");
	auto meshCmdIndexBuffer = m_objectManager.getBuffer("meshCmdIndexBuffer");

	gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	gl::BufferBindingManager::bindBaseUniform(1, lightDataBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
	gl::BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(5, pShadowTransformBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(6, pVoxelBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(7, pVoxelTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(10, renderSettingsBufferBlock->getId());

	gl::BufferBindingManager::bindBaseShaderStorage(0, transformBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(1, materialBlockBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(2, pTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(3, shaderStorageConetracePixelListBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(4, shaderStoragePixelListCmdBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(5, lightCmdBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(6, meshCmdIndexBuffer->getId());
}

void RenderAspect::clearTextures()
{
	m_pColdData->m_voxelRenderer.clearTextures();

	m_pColdData->m_ambientColorTexture.clearImage(0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);

	m_shadowRenderer->clearData();
}

void RenderAspect::clearBuffers()
{
	auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");

	auto mapped = shaderStoragePixelListCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
	mapped->count = 0;
}

void RenderAspect::createGBuffer(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer)
{
	auto meshParamBuffer = m_objectManager.getBuffer("meshParamBuffer");

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer);
	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, *meshParamBuffer);
	vx::gl::StateManager::bindFrameBuffer(m_gbufferFB);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("create_gbuffer.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	vx::gl::StateManager::bindVertexArray(vao);

	glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));
}

void RenderAspect::voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, u32 count)
{
	auto voxelFB = m_objectManager.getFramebuffer("voxelFB");

	auto pipeline = m_shaderManager.getPipeline("voxelize.pipe");
	auto geomShader = pipeline->getGeometryShader();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::bindFrameBuffer(*voxelFB);

	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	vx::gl::StateManager::bindPipeline(*pipeline);
	vx::gl::StateManager::bindVertexArray(vao);
	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer.getId());

	glProgramUniform1i(geomShader, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, 128, 128);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));

	glProgramUniform1i(geomShader, 0, 1);
	vx::gl::StateManager::setViewport(0, 0, 64, 64);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));

	glProgramUniform1i(geomShader, 0, 2);
	vx::gl::StateManager::setViewport(0, 0, 32, 32);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));

	glProgramUniform1i(geomShader, 0, 3);
	vx::gl::StateManager::setViewport(0, 0, 16, 16);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
}

void RenderAspect::voxelDebug()
{
	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	m_pColdData->m_voxelRenderer.debug(*emptyVao, m_resolution);
}

void RenderAspect::createConeTracePixelList()
{
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	auto emptyVao = m_objectManager.getVertexArray("emptyVao");

	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("createConeTraceList.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::coneTrace()
{
	auto emptyVao = m_objectManager.getVertexArray("emptyVao");

	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");
	auto pPipeline = m_shaderManager.getPipeline("coneTrace.pipe");

	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, shaderStoragePixelListCmdBuffer->getId());

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	glDrawArraysIndirect(GL_POINTS, 0);
}

void RenderAspect::blurAmbientColor()
{
	auto emptyVao = m_objectManager.getVertexArray("emptyVao");

	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

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

	CpuProfiler::render();

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

void RenderAspect::handleIngameEvent(const vx::Event &evt)
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

		vx::Event e;
		e.arg1 = evt.arg1;
		e.arg2.u32 = (u32)gpuIndex;
		e.code = (u32)IngameEvent::Created_Actor_GPU;
		e.type = vx::EventType::Ingame_Event;

		evtManager->addEvent(e);
	}
}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	*m = m_renderContext.getProjectionMatrix();
}

u16 RenderAspect::addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material, const Scene* pScene)
{
	vx::gl::DrawElementsIndirectCommand drawCmd;
	u32 cmdIndex = 0;
	auto gpuIndex = m_sceneRenderer.addActorToBuffer(transform, mesh, material, pScene, &drawCmd, &cmdIndex);

	m_shadowRenderer->updateDrawCmd(drawCmd, cmdIndex);

	auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);

	return gpuIndex;
}

const Font& RenderAspect::getProfilerFont() const
{
	return m_pColdData->m_font;
}