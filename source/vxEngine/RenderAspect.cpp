#include "RenderAspect.h"
#include "GpuStructs.h"
#include <vxLib/gl/gl.h>
#include <vxLib/gl/Debug.h>
#include <vxLib/gl/StateManager.h>
#include "BufferBindingManager.h"
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
#include "CapabilityEnablePolygonOffsetFill.h"
#include "CapabilityDisablePolygonOffsetFill.h"
#include "CapabilitySettingPolygonOffset.h"
#include "CapabilityFactory.h"
#include "CreateActorData.h"
#include "Locator.h"
#include "EventManager.h"

RenderAspect* g_renderAspect{ nullptr };

namespace
{
	void __stdcall debugCallback()
	{
		assert(false);
	}
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
		desc.size = m_resolution.x * m_resolution.y * sizeof(F32) * 4;
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Read;
		m_pColdData->m_screenshotBuffer.create(desc);
	}

	/*vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Pixel_Pack_Buffer;
	desc.size = m_pColdData->m_windowResolution.x * m_pColdData->m_windowResolution.y * sizeof(F32) * 4;
	desc.immutable = 1;
	desc.flags = vx::gl::BufferStorageFlags::Read;

	//m_videoBuffers[0].create(desc);
//	m_videoBuffers[1].create(desc);
	//m_videoBuffers[2].create(desc);*/

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

		m_bufferManager.createBuffer("CameraBufferStatic", cameraDesc);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(ShadowTransformBlock);
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;

		m_bufferManager.createBuffer("ShadowTransformBuffer", desc);
	}

	{
		U64 handles[8];
		handles[0] = m_pColdData->m_gbufferAlbedoSlice.getTextureHandle();
		handles[1] = m_pColdData->m_gbufferNormalSlice.getTextureHandle();
		handles[2] = m_pColdData->m_gbufferSurfaceSlice.getTextureHandle();
		handles[3] = m_pColdData->m_gbufferTangentSlice.getTextureHandle();
		handles[4] = m_pColdData->m_gbufferDepthTexture.getTextureHandle();
		handles[5] = m_pColdData->m_aabbTexture.getTextureHandle();
		handles[6] = m_pColdData->m_shadowTexture.getTextureHandle();
		handles[7] = m_pColdData->m_ambientColorTexture.getTextureHandle();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(handles);
		desc.immutable = 1;
		desc.pData = handles;

		m_bufferManager.createBuffer("UniformTextureBuffer", desc);
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

	{
		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::DEPTH32F;
		desc.type = vx::gl::TextureType::Texture_Cubemap;
		desc.size = vx::ushort3(s_shadowMapResolution, s_shadowMapResolution, 6);
		desc.miplevels = 1;
		desc.sparse = 0;
		m_pColdData->m_shadowTexture.create(desc);

		m_pColdData->m_shadowTexture.setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
		m_pColdData->m_shadowTexture.setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE, vx::gl::TextureWrapMode::CLAMP_TO_EDGE);

		glTextureParameteri(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		//float ones[] = { 0, 0, 0, 0 };
		//glTextureParameterfv(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_BORDER_COLOR, ones);

		m_pColdData->m_shadowTexture.makeTextureResident();
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
		m_shadowFB.create();
		m_shadowFB.attachTexture(vx::gl::Attachment::Depth, m_pColdData->m_shadowTexture, 0);
		glNamedFramebufferDrawBuffer(m_shadowFB.getId(), GL_NONE);

		auto status = m_shadowFB.checkStatus();
		assert(status == GL_FRAMEBUFFER_COMPLETE);
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

bool RenderAspect::initialize(const std::string &dataDir, const RenderAspectDescription &desc)
{
	vx::gl::ContextDescription contextDesc = vx::gl::ContextDescription::create(*desc.window, desc.resolution, desc.fovRad, desc.z_near, desc.z_far, 4, 5, desc.vsync, desc.debug);
	if (!m_renderContext.initialize(contextDesc))
		return false;

	auto result = initializeImpl(dataDir, desc.resolution, desc.debug, desc.pAllocator);

	if (result)
		bindBuffers();

	return result;
}

bool RenderAspect::initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug, vx::StackAllocator *pAllocator)
{
	m_pColdData = std::make_unique<ColdData>();
	m_resolution = windowResolution;

	m_allocator = vx::StackAllocator(pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);
	
	if (debug)
	{
		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(::debugCallback);
		vx::gl::Debug::enableCallback(true);
	}

	vx::gl::StateManager::enable(vx::gl::Capabilities::Framebuffer_sRGB);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::setViewport(0, 0, windowResolution.x, windowResolution.y);

	m_bufferManager.initialize(50, &m_allocator);

	m_camera.setPosition(0, 2.5f, 15);

	if (!m_shaderManager.initialize(dataDir))
	{
		puts("Error initializing Shadermanager");
		return false;
	}

	createTextures();
	createFrameBuffers();

	createUniformBuffers();
	createBuffers();

#if _VX_PROFILER
	/*if (pProfiler)
	{
		auto textureIndex = *m_pColdData->m_texturesGPU.find(glGetTextureHandleARB(m_pColdData->m_font.getTextureEntry().getTextureId()));
		if (!pProfiler->initialize(&m_pColdData->m_font, m_shaderManager.getPipeline("text.pipe"), textureIndex, m_pColdData->m_windowResolution, pAllocator))
			return false;
	}

	if (pGraph)
	{
		pGraph->initialize(m_shaderManager, targetMs);
	}*/
#else
	VX_UNREFERENCED_PARAMETER(pAllocator);
#endif

	m_sceneRenderer.initialize(10, &m_bufferManager, pAllocator);
	m_voxelRenderer.initialize(128, m_shaderManager, &m_bufferManager);

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
	m_pColdData->m_capabilityManager.createCapabilitySettingsPolygonOffsetFill(2.5f, 10.0f, "polygonOffsetFillShadowMap");
	auto capEnablePolygonOffsetFill = m_pColdData->m_capabilityManager.createEnablePolygonOffsetFill("enablePolygonOffsetFillShadowMap", "polygonOffsetFillShadowMap");
	auto capEnableDepthTest = m_pColdData->m_capabilityManager.createEnableDepthTest("enableDepthTest");
	auto capDisablePolygonOffsetFill = m_pColdData->m_capabilityManager.createDisablePolygonOffsetFill("disablePolygonOffsetFill");

	auto &cmdBuffer = m_sceneRenderer.getCmdBuffer();
	auto &meshVao = m_sceneRenderer.getMeshVao();

	Graphics::DrawCommandDescription drawCmdDesc;
	drawCmdDesc.dataType = vx::gl::DataType::Unsigned_Int;
	drawCmdDesc.primitiveMode = Graphics::PrimitiveMode::Triangles;
	drawCmdDesc.indirectBuffer = cmdBuffer.getId();
	drawCmdDesc.renderMode = Graphics::RenderMode::MultiDrawElementsIndirect;

	Graphics::RenderPassDescription desc;
	desc.drawCmdDesc = drawCmdDesc;
	desc.resolution = vx::uint2(s_shadowMapResolution, s_shadowMapResolution);
	desc.vao = meshVao.getId();
	desc.fbo = m_shadowFB.getId();
	desc.clearBits = GL_DEPTH_BUFFER_BIT;
	desc.pipeline = m_shaderManager.getPipeline("shadow.pipe")->getId();

	Graphics::RenderPass renderPassCreateShadowMap;
	renderPassCreateShadowMap.initialize(desc);

	m_renderStageCreateShadowMap.pushRenderPass("createShadowMap", renderPassCreateShadowMap);
	m_renderStageCreateShadowMap.attachCapabilityBegin("createShadowMap", capEnablePolygonOffsetFill);
	m_renderStageCreateShadowMap.attachCapabilityBegin("createShadowMap", capEnableDepthTest);
	m_renderStageCreateShadowMap.attachCapabilityEnd("createShadowMap", capDisablePolygonOffsetFill);
}

bool RenderAspect::initializeProfiler(GpuProfiler* gpuProfiler, vx::StackAllocator* allocator)
{
	auto fontHandle = glGetTextureHandleARB(m_pColdData->m_font.getTextureEntry().getTextureId());

	return m_sceneRenderer.initializeProfiler(m_pColdData->m_font, fontHandle, m_resolution, m_shaderManager, gpuProfiler, allocator);
}

void RenderAspect::shutdown(const HWND hwnd)
{
	m_bufferManager.shutdown();
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
	for (auto &it : m_tasks)
	{
		switch (it.type)
		{
		case RenderUpdateTask::Type::UpdateCamera:
			taskUpdateCamera();
			break;
		case RenderUpdateTask::Type::UpdateDynamicTransforms:
			taskUpdateDynamicTransforms(it.ptr);
			break;
		case RenderUpdateTask::Type::CreateActorGpuIndex:
			taskCreateActorGpuIndex(it.ptr);
			break;
		case RenderUpdateTask::Type::TakeScreenshot:
			taskTakeScreenshot();
			break;
		case RenderUpdateTask::Type::LoadScene:
			taskLoadScene(it.ptr);
			break;
		case RenderUpdateTask::Type::ToggleRenderMode:
			taskToggleRenderMode();
			break;
		default:
			break;
		}
	}
	m_tasks.clear();
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

void RenderAspect::taskLoadScene(void* p)
{
	Scene* pScene = (Scene*)p;
	vx::verboseChannelPrintF(0, dev::Channel_Render, "Loading Scene into Render");
	m_sceneRenderer.loadScene(*pScene, m_bufferManager);
	m_pScene = pScene;

	auto count = m_sceneRenderer.getMeshInstanceCount();
	m_renderStageCreateShadowMap.setDrawCountAll(count);
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

void RenderAspect::taskCreateActorGpuIndex(void* p)
{
	auto evtManager = Locator::getEventManager();

	CreateActorData* data = (CreateActorData*)p;
	auto gpuIndex = addActorToBuffer(data->transform, data->mesh, data->material, data->pScene);

	printf("gpuIndex: %u\n", gpuIndex);

	Event e;
	e.arg1 = data->index;
	e.arg2 = (U32)gpuIndex;
	e.code = (U32)IngameEvent::Created_Actor_GPU;
	e.type = EventType::Ingame_Event;

	evtManager->addEvent(e);

	delete(data);
}

void RenderAspect::taskUpdateDynamicTransforms(void* p)
{
	RenderUpdateDataTransforms data = *(RenderUpdateDataTransforms*)p;
	delete(p);

	for (U32 i = 0; i < data.count; ++i)
	{
		m_sceneRenderer.updateTransform(data.transforms[i], data.indices[i]);
	}

	delete[]data.indices;
	_aligned_free(data.transforms);
}

void RenderAspect::render(GpuProfiler* gpuProfiler)
{
	gpuProfiler->pushGpuMarker("clear");
	clearTextures();
	clearBuffers();
	gpuProfiler->popGpuMarker();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

	auto &cmdBuffer = m_sceneRenderer.getCmdBuffer();
	auto meshCount = m_sceneRenderer.getMeshInstanceCount();
	auto &meshVao = m_sceneRenderer.getMeshVao();

	gpuProfiler->pushGpuMarker("shadow mapping");
	createShadowMap(meshVao,cmdBuffer, meshCount);
	gpuProfiler->popGpuMarker();

	gpuProfiler->pushGpuMarker("gbuffer");
	createGBuffer(meshVao,cmdBuffer, meshCount);
	gpuProfiler->popGpuMarker();

		glMemoryBarrier(GL_ALL_BARRIER_BITS);
		gpuProfiler->pushGpuMarker("voxelize");
		voxelize(meshVao, cmdBuffer, meshCount);
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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_renderContext.swapBuffers();
}

void RenderAspect::readFrame()
{
	/*m_currentReadBuffer = (m_currentReadBuffer + 1) % 3;
	m_currentWriteBuffer = (m_currentWriteBuffer + 1) % 3;

	m_videoBuffers[m_currentWriteBuffer].bind();
	glReadPixels(0, 0, m_resolution.x, m_resolution.x, GL_RGBA, GL_FLOAT, 0);*/
}

void RenderAspect::getFrameData(vx::float4a* dst)
{
	//glGetNamedBufferSubData(m_videoBuffers[m_currentReadBuffer].getId(), 0, sizeof(vx::float4a) * m_resolution.x*m_resolution.y, dst);
}

void RenderAspect::bindBuffers()
{
	auto pShadowTransformBuffer = m_bufferManager.getBuffer("ShadowTransformBuffer");
	auto pTextureBuffer = m_bufferManager.getBuffer("TextureBuffer");
	auto pUniformTextureBuffer = m_bufferManager.getBuffer("UniformTextureBuffer");
	auto pCameraBufferStatic = m_bufferManager.getBuffer("CameraBufferStatic");

	BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
	BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());
	BufferBindingManager::bindBaseUniform(5, pShadowTransformBuffer->getId());

	BufferBindingManager::bindBaseShaderStorage(2, pTextureBuffer->getId());

	m_sceneRenderer.bindBuffers();
	m_voxelRenderer.bindBuffers(m_bufferManager);
}

void RenderAspect::clearTextures()
{
	m_voxelRenderer.clearTextures();

	m_pColdData->m_ambientColorTexture.clearImage(0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);
}

void RenderAspect::clearBuffers()
{
}

void RenderAspect::createShadowMap(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count)
{
	//vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
	//vx::gl::StateManager::enable(vx::gl::Capabilities::Polygon_Offset_Fill);

	//glPolygonOffset(2.5f, 10.0f);

	m_renderStageCreateShadowMap.draw();

	//vx::gl::StateManager::disable(vx::gl::Capabilities::Polygon_Offset_Fill);
}

void RenderAspect::createGBuffer(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count)
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindFrameBuffer(m_gbufferFB);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("create_gbuffer.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	vx::gl::StateManager::bindVertexArray(vao);

	cmdBuffer.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));
}

void RenderAspect::voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count)
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

	U32 src = 0;
	U32 dst = 1;
	F32 pixelDistance = 2.0f;

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

void RenderAspect::keyPressed(U16 key)
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
	auto p = m_pColdData->m_screenshotBuffer.map<U8>(vx::gl::Map::Read_Only);
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
		task.ptr = pScene;

		queueUpdateTask(task);
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
		e.arg2 = (U32)gpuIndex;
		e.code = (U32)IngameEvent::Created_Actor_GPU;
		e.type = EventType::Ingame_Event;

		evtManager->addEvent(e);
	}
}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	*m = m_renderContext.getProjectionMatrix();
}

U16 RenderAspect::addActorToBuffer(const vx::Transform &transform, const vx::StringID64 &mesh, const vx::StringID64 &material, const Scene* pScene)
{
	return m_sceneRenderer.addActorToBuffer(transform, mesh, material, pScene);
}