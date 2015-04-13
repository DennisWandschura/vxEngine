#include "RenderAspect.h"
#include "BufferBlocks.h"
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
		desc.size = m_pColdData->m_windowResolution.x * m_pColdData->m_windowResolution.y * sizeof(F32) * 4;
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
		U64 handles[9];
		handles[0] = m_pColdData->m_gbufferAlbedoSlice.getTextureHandle();
		handles[1] = m_pColdData->m_gbufferNormalSlice.getTextureHandle();
		handles[2] = m_pColdData->m_gbufferSurfaceSlice.getTextureHandle();
		handles[3] = m_pColdData->m_gbufferTangentSlice.getTextureHandle();
		handles[4] = m_pColdData->m_gbufferDepthTexture.getTextureHandle();
		handles[5] = m_pColdData->m_aabbTexture.getTextureHandle();
		handles[6] = m_pColdData->m_shadowTexture.getTextureHandle();
		handles[7] = m_pColdData->m_ambientColorTexture.getTextureHandle();
		handles[8] = m_pColdData->m_ambientColorBlurTexture.getTextureHandle();

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
		desc.size = vx::ushort3(m_pColdData->m_windowResolution.x, m_pColdData->m_windowResolution.y, 1);
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
		desc.size = vx::ushort3(m_pColdData->m_windowResolution.x, m_pColdData->m_windowResolution.y, 1);
		m_pColdData->m_gbufferDepthTexture.create(desc);
		m_pColdData->m_gbufferDepthTexture.makeTextureResident();
	}

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::RGBA16F;
		desc.size = vx::ushort3(1920, 1080, 1);
		desc.miplevels = 1;
		m_pColdData->m_ambientColorTexture.create(desc);
		m_pColdData->m_ambientColorTexture.makeTextureResident();

		m_pColdData->m_ambientColorBlurTexture.create(desc);
		m_pColdData->m_ambientColorBlurTexture.makeTextureResident();
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
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::R8;
		desc.size = vx::ushort3(4096, 4096, 1);
		desc.miplevels = 1;
		m_pColdData->m_testTexture.create(desc);
	}

	{
		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::DEPTH32;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.size = vx::ushort3(s_shadowMapResolution, s_shadowMapResolution, 1);
		desc.miplevels = 1;
		desc.sparse = 0;
		m_pColdData->m_shadowTexture.create(desc);

		m_pColdData->m_shadowTexture.setFilter(vx::gl::TextureFilter::LINEAR, vx::gl::TextureFilter::LINEAR);
		m_pColdData->m_shadowTexture.setWrapMode2D(vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER);

		glTextureParameteri(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glTextureParameteri(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		float ones[] = { 0, 0, 0, 0 };
		glTextureParameterfv(m_pColdData->m_shadowTexture.getId(), GL_TEXTURE_BORDER_COLOR, ones);

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

		m_blurFB.create();
		m_blurFB.attachTexture(vx::gl::Attachment::Color0, m_pColdData->m_ambientColorBlurTexture, 0);
		glNamedFramebufferDrawBuffer(m_blurFB.getId(), GL_COLOR_ATTACHMENT0);
	}
}

bool RenderAspect::initialize(const std::string &dataDir, const RenderAspectDesc &desc)
{
	vx::gl::ContextDescription contextDesc = vx::gl::ContextDescription::create(*desc.window, desc.resolution, desc.fovRad, desc.z_near, desc.z_far, 4, 5, desc.vsync, desc.debug);
	if (!m_renderContext.initialize(contextDesc))
		return false;

	return initializeImpl(dataDir, desc.resolution, desc.debug, desc.targetMs, desc.pAllocator, desc.pProfiler, desc.pGraph);
}

bool RenderAspect::initializeImpl(const std::string &dataDir, const vx::uint2 &windowResolution, bool debug,
	F32 targetMs, vx::StackAllocator *pAllocator, Profiler2 *pProfiler, ProfilerGraph* pGraph)
{
	m_pColdData = std::make_unique<ColdData>();
	m_pColdData->m_windowResolution = windowResolution;

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

	/*{
		TextureFile fontTexture;
		if (!fontTexture.loadFromFile((dataDir + "textures/verdana.png").c_str()))
			return false;

		auto ref = m_pColdData->m_textureManager.load(fontTexture, 1, 1);

		FontAtlas fontAtlas;
		if (!fontAtlas.loadFromFile((dataDir + "fonts/meta/VerdanaRegular.sdff").c_str()))
			return false;

		m_pColdData->m_font = Font(std::move(ref), std::move(fontAtlas));
	}*/

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
	VX_UNREFERENCED_PARAMETER(pProfiler);
	VX_UNREFERENCED_PARAMETER(pGraph);
	VX_UNREFERENCED_PARAMETER(targetMs);
	VX_UNREFERENCED_PARAMETER(pAllocator);
#endif

	m_sceneRenderer.initialize(10, &m_bufferManager, pAllocator);
	m_navMeshRenderer.initialize(m_shaderManager);
	m_voxelRenderer.initialize(m_shaderManager, &m_bufferManager);
	bindBuffers();

	return true;
}

/*void RenderAspect::writeMeshToVertexBuffer(const vx::StringID64 &meshSid, const vx::Mesh* pMesh, U32 *vertexOffsetGpu, U32 *indexOffsetGpu)
{
	auto marker = m_scratchAllocator.getMarker();
	SCOPE_EXIT
	{
		m_scratchAllocator.clear(marker);
	};

	auto vertexCount = pMesh->getVertexCount();
	auto vertexSizeBytes = sizeof(VertexPNTUV) * vertexCount;
	VertexPNTUV* pVertices = (VertexPNTUV*)m_scratchAllocator.allocate(vertexSizeBytes);

	auto indexCount = pMesh->getIndexCount();
	U32 indexSizeBytes = sizeof(U32) * indexCount;
	U32* pIndices = (U32*)m_scratchAllocator.allocate(indexSizeBytes);

	U32 offsetBytes = *vertexOffsetGpu * sizeof(VertexPNTUV);
	U32 offsetIndicesBytes = sizeof(U32) * (*indexOffsetGpu);

	U32 tmpOffset = 0;
	writeMeshToBuffer(meshSid, pMesh, pVertices, pIndices, &tmpOffset, &tmpOffset, vertexOffsetGpu, indexOffsetGpu);

	auto pGpuVertices = m_pColdData->m_meshVbo.mapRange<VertexPNTUV>(offsetBytes, vertexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuVertices.get(), pVertices, vertexSizeBytes);

	auto pGpuIndices = m_pColdData->m_meshIbo.mapRange<U32>(offsetIndicesBytes, indexSizeBytes, vx::gl::MapRange::Write);
	memcpy(pGpuIndices.get(), pIndices, indexSizeBytes);
}*/

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
		case RenderUpdateTask::Type::TakeScreenshot:
			taskTakeScreenshot();
			break;
		case RenderUpdateTask::Type::LoadScene:
			taskLoadScene(it.ptr);
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
	m_sceneRenderer.loadScene(*pScene, m_bufferManager, &m_navMeshRenderer);
	m_pScene = pScene;
}

void RenderAspect::render()
{
	clearTextures();
	clearBuffers();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

	auto &cmdBuffer = m_sceneRenderer.getCmdBuffer();
	auto meshCount = m_sceneRenderer.getMeshInstanceCount();
	auto &meshVao = m_sceneRenderer.getMeshVao();

	createShadowMap(meshVao,cmdBuffer, meshCount);

	createGBuffer(meshVao,cmdBuffer, meshCount);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	voxelize(meshVao, cmdBuffer, meshCount);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);

	vx::gl::StateManager::bindFrameBuffer(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	coneTrace();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	blurAmbientColor();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	renderFinalImage();

	//voxelDebug();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	// nav nodes
	if (dev::g_showNavGraph != 0)
	{
		renderNavGraph();
	}

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_renderContext.swapBuffers();
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
	vx::gl::StateManager::setViewport(0, 0, s_shadowMapResolution, s_shadowMapResolution);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Polygon_Offset_Fill);

	glPolygonOffset(2.5f, 10.0f);

	vx::gl::StateManager::bindFrameBuffer(m_shadowFB);
	glClear(GL_DEPTH_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("shadow.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(vao);

	cmdBuffer.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, count, sizeof(vx::gl::DrawElementsIndirectCommand));

	vx::gl::StateManager::disable(vx::gl::Capabilities::Polygon_Offset_Fill);
}

void RenderAspect::createGBuffer(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, U32 count)
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
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
	m_voxelRenderer.debug(m_emptyVao);
}

/*void RenderAspect::createTrianglesAndAABBs()
{
	vx::gl::StateManager::setViewport(0, 0, 1024, 2);
	vx::gl::StateManager::setClearColor(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
	vx::gl::StateManager::bindFrameBuffer(m_aabbFB);
	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

	glBlendEquation(GL_MIN);
	glBlendFunc(GL_SRC_COLOR, GL_ZERO);

	auto pPipeline = m_shaderManager.getPipeline("create_triangles_aabb.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

	m_commandBlock.bind();
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
}*/

void RenderAspect::coneTrace()
{
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);
	glClear(GL_COLOR_BUFFER_BIT);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("coneTrace.pipe");
	auto fs = pPipeline->getFragmentShader();

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
	vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glDrawArrays(GL_POINTS, 0, 1);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
}

void RenderAspect::blurAmbientColor()
{
	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
	vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

	vx::gl::StateManager::bindFrameBuffer(m_blurFB);
	glClear(GL_COLOR_BUFFER_BIT);

	auto pPipeline = m_shaderManager.getPipeline("blurpass1.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());

	glDrawArrays(GL_POINTS, 0, 1);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);
	glClear(GL_COLOR_BUFFER_BIT);

	pPipeline = m_shaderManager.getPipeline("blurpass2.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::renderFinalImage()
{
	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("draw_final_image.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::setViewport(0, 0, 1920, 1080);
	vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::renderProfiler(Profiler2* pProfiler, ProfilerGraph* pGraph)
{
	/*vx::gl::StateManager::setViewport(0, 0, 1920, 1080);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	pProfiler->render();
	pGraph->render();

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);*/
}

void RenderAspect::renderNavGraph()
{
	m_navMeshRenderer.render();
}

/*U16 RenderAspect::addActorToBuffer(const vx::Transform &transform, const vx::StringID64 &mesh, const vx::StringID64 &material, const Scene* pScene)
{
	auto itMesh = m_meshEntries.find(mesh);
	if (itMesh == m_meshEntries.end())
	{
		auto &sceneMeshes = pScene->getMeshes();
		auto itSceneMesh = sceneMeshes.find(mesh);

		U32 vertexOffset = m_meshVertexCountDynamic + s_maxVerticesStatic;
		U32 indexOffset = m_meshIndexCountDynamic + s_maxIndicesStatic;
		writeMeshToVertexBuffer(mesh, (*itSceneMesh), &vertexOffset, &indexOffset);

		m_meshVertexCountDynamic += (*itSceneMesh)->getVertexCount();
		m_meshIndexCountDynamic += (*itSceneMesh)->getIndexCount();

		itMesh = m_meshEntries.find(mesh);
	}

	auto pMaterial = m_fileAspect.getMaterial(material);

	// add material
	auto itMaterial = m_materialIndices.find(pMaterial);
	if (itMaterial == m_materialIndices.end())
	{
		createMaterial(pMaterial);

		writeMaterialToBuffer(pMaterial, m_materialCount);

		itMaterial = m_materialIndices.insert(pMaterial, m_materialCount);

		++m_materialCount;
	}

	// create transform and draw command
	auto materialIndex = *itMaterial;

	U32 elementId = s_meshMaxInstancesStatic + m_meshInstanceCountDynamic;
	U16 index = m_meshInstancesCountTotal;

	writeTransform(transform, elementId);
	writeMeshInstanceIdBuffer(elementId, materialIndex);
	writeMeshInstanceToCommandBuffer(*itMesh, index, elementId);

	++m_meshInstanceCountDynamic;

	m_meshInstancesCountTotal = m_meshInstanceCountStatic + m_meshInstanceCountDynamic;

	return elementId;
}*/

/*U16 RenderAspect::getActorGpuIndex()
{

}*/

void RenderAspect::takeScreenshot()
{
	const auto resDim = m_pColdData->m_windowResolution.x * m_pColdData->m_windowResolution.y;
	const auto pixelBufferSizeBytes = sizeof(vx::float4a) * resDim;

	m_pColdData->m_screenshotBuffer.bind();
	glReadPixels(0, 0, m_pColdData->m_windowResolution.x, m_pColdData->m_windowResolution.y, GL_RGBA, GL_FLOAT, 0);

	auto pScreenshotData = (vx::float4a*)_aligned_malloc(pixelBufferSizeBytes, 16);
	auto p = m_pColdData->m_screenshotBuffer.map<U8>(vx::gl::Map::Read_Only);
	memcpy(pScreenshotData, p.get(), pixelBufferSizeBytes);
	p.unmap();

	ScreenshotFactory::writeScreenshotToFile(m_pColdData->m_windowResolution, pScreenshotData);
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
		NavGraph* pGraph = (NavGraph*)evt.arg1.ptr;

		m_navMeshRenderer.updateBuffer(*pGraph);
	}
}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	*m = m_renderContext.getProjectionMatrix();
}