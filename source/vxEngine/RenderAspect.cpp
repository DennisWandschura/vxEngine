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
#include <vxEngineLib/debugPrint.h>
#include "GpuProfiler.h"
#include "CreateActorData.h"
#include "Locator.h"
#include <vxEngineLib/EventManager.h>
#include "Graphics/Renderer.h"
#include "Graphics/ShadowRenderer.h"
#include "Graphics/Segment.h"
#include <vxLib/Window.h>
#include "CpuProfiler.h"
#include "Graphics/CommandListFactory.h"
#include <vxEngineLib/FileEvents.h>
#include <UniformReflectionBuffer.h>

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
		cameraDesc.size = sizeof(UniformCameraBufferBlock);
		cameraDesc.immutable = 1;
		cameraDesc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
		m_cameraBuffer.create(cameraDesc);
	}

	{
		UniformCameraBufferStaticBlock cameraBlockStatic;
		cameraBlockStatic.invProjectionMatrix = vx::MatrixInverse(m_renderContext.getProjectionMatrix());
		cameraBlockStatic.projectionMatrix = m_renderContext.getProjectionMatrix();
		cameraBlockStatic.orthoMatrix = m_renderContext.getOrthoMatrix();

		vx::gl::BufferDescription cameraDesc;
		cameraDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		cameraDesc.immutable = 1;
		cameraDesc.size = sizeof(UniformCameraBufferStaticBlock);
		cameraDesc.flags = vx::gl::BufferStorageFlags::None;
		cameraDesc.pData = &cameraBlockStatic;

		m_objectManager.createBuffer("CameraBufferStatic", cameraDesc);
	}

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(ShadowTransformBufferBlock);
		desc.immutable = 1;
		desc.flags = vx::gl::BufferStorageFlags::Write;

		m_objectManager.createBuffer("ShadowTransformBuffer", desc);
	}

	{
		auto volumetrixTextuzre = m_objectManager.getTexture("volumetricFogTexture");

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
		data.u_volumetricTexture = volumetrixTextuzre->getTextureHandle();

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

	{

		auto getTransform = [](const __m128 &position, f32 radius, UniformReflectionBufferBlock *data)
		{
			auto projectionMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(90.0f), 1.0f, 0.1f, radius);

			vx::mat4 viewMatrices[6];
			// X+
			vx::float4 up = { 0, -1, 0, 0 };
			vx::float4 dir = { 1, 0, 0, 0 };
			viewMatrices[0] = vx::MatrixLookToRH(position, vx::loadFloat4(dir), vx::loadFloat4(up));
			// X-
			up = { 0, -1, 0, 0 };
			dir = { -1, 0, 0, 0 };
			viewMatrices[1] = vx::MatrixLookToRH(position, vx::loadFloat4(dir), vx::loadFloat4(up));
			// Y+
			up = { 0, 0, 1, 0 };
			dir = vx::float4(0, 1, 0, 0);
			viewMatrices[3] = vx::MatrixLookToRH(position, vx::loadFloat4(dir), vx::loadFloat4(up));
			// Y-
			up = { 0, 0, -1, 0 };
			dir = vx::float4(0, -1, 0, 0);
			viewMatrices[2] = vx::MatrixLookToRH(position, vx::loadFloat4(dir), vx::loadFloat4(up));
			// Z+
			up = { 0, -1, 0, 0 };
			dir = vx::float4(0, 0, 1, 0);
			viewMatrices[4] = vx::MatrixLookToRH(position, vx::loadFloat4(dir), vx::loadFloat4(up));
			// Z-
			up = { 0, -1, 0, 0 };
			dir = vx::float4(0, 0, -1, 0);
			viewMatrices[5] = vx::MatrixLookToRH(position, vx::loadFloat4(dir), vx::loadFloat4(up));

			data->position = position;
			data->projectionMatrix = projectionMatrix;
			for (u32 i = 0; i < 6; ++i)
			{
				data->pvMatrix[i] = projectionMatrix * viewMatrices[i];
			}
		};

		__m128 position = { -9, 1.5f, 1, 0 };
		UniformReflectionBufferBlock block;
		getTransform(position, 2.0f, &block);

		auto texture = m_objectManager.getTexture("reflectionTexture");

		block.texture = texture->getTextureHandle();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(UniformReflectionBufferBlock);
		desc.immutable = 1;
		desc.pData = &block;

		m_objectManager.createBuffer("UniformReflectionBuffer", desc);
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

		desc.format = vx::gl::TextureFormat::RGB16F;
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
		desc.type = vx::gl::TextureType::Texture_3D;
		desc.format = vx::gl::TextureFormat::R8;
		desc.size = vx::ushort3(64, 64, 64);
		desc.miplevels = 1;

		auto sid = m_objectManager.createTexture("volumetricFogTexture", desc);
		auto texture = m_objectManager.getTexture(sid);
		texture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER);

		union IntFloat
		{
			int i;
			float f;
		};

		//const auto dataSize = sizeof(u8) * 64 * 64 * 64;
		const auto dataCount = 64 * 64 * 64;
		auto data = vx::make_unique<s8[]>(dataCount);
		for (u32 i = 0; i < dataCount; ++i)
		{
			//IntFloat intFloat;
			//intFloat.f = 0.0f;

			data[i] = 0;
		}

		for (u32 z = 0; z < 32; ++z)
		{
			for (u32 y = 0; y < 64; ++y)
			{
				for (u32 x = 0; x < 37; ++x)
				{
					auto index = x + 64 * (y + z * 64);

					//IntFloat intFloat;
					//	intFloat.f = 1.0f;

					data[index] = 1;
				}
			}
		}

		vx::gl::TextureSubImageDescription subImgDesc;
		subImgDesc.dataType = vx::gl::DataType::Byte;
		subImgDesc.miplevel = 0;
		subImgDesc.offset = vx::uint3(0, 0, 0);
		subImgDesc.p = data.get();
		subImgDesc.size = vx::uint3(64, 64, 64);
		texture->subImage(subImgDesc);

		texture->makeTextureResident();
	}

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_Cubemap;
		desc.format = vx::gl::TextureFormat::RGB16F;
		desc.size = vx::ushort3(1024, 1024, 6);
		desc.miplevels = 1;

		m_objectManager.createTexture("reflectionTexture", desc);

		auto texture = m_objectManager.getTexture("reflectionTexture");
		texture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER);
		texture->makeTextureResident();

		desc.format = vx::gl::TextureFormat::DEPTH16;
		m_objectManager.createTexture("reflectionTextureDepth", desc);
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

	{
		auto colorTexture = m_objectManager.getTexture("reflectionTexture");
		auto depthTexture = m_objectManager.getTexture("reflectionTextureDepth");

		auto sid = m_objectManager.createFramebuffer("reflectionFbo");
		auto fbo = m_objectManager.getFramebuffer(sid);

		fbo->attachTexture(vx::gl::Attachment::Color0, *colorTexture, 0);
		fbo->attachTexture(vx::gl::Attachment::Depth, *depthTexture, 0);

		glNamedFramebufferDrawBuffer(fbo->getId(), GL_COLOR_ATTACHMENT0);
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
	if (!result)
		return false;

	createFrame();

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

	m_objectManager.initialize(50, 20, 20, 20, &m_allocator);

	m_camera.setPosition(0, 2.5f, 15);

	//m_shaderManager.addParameter("lightInvo", 6u);
	//m_shaderManager.addParameter("maxLightCount", 5u);
	if (!m_shaderManager.initialize(dataDir, &m_allocator, true))
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

	m_sceneRenderer.initialize(10, &m_objectManager, pAllocator);
	m_pColdData->m_voxelRenderer.initialize(128, m_shaderManager, &m_objectManager);

	{
		auto pShadowRenderer = vx::make_unique<Graphics::ShadowRenderer>();
		pShadowRenderer->initialize();

		m_shadowRenderer = std::move(pShadowRenderer);
	}

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

	return true;
}

void RenderAspect::createFrame()
{
	Graphics::CommandList cmdList;
	cmdList.initialize();
	m_shadowRenderer->getCommandList(&cmdList);

	m_frame.pushCommandList(std::move(cmdList));
}

bool RenderAspect::initializeProfiler(GpuProfiler* gpuProfiler, vx::StackAllocator* allocator)
{
	auto fontHandle = glGetTextureHandleARB(m_pColdData->m_font.getTextureEntry().getTextureId());

	return m_sceneRenderer.initializeProfiler(m_pColdData->m_font, fontHandle, m_resolution, m_shaderManager, gpuProfiler, allocator);
}

void RenderAspect::shutdown(const HWND hwnd)
{
	m_tasks.clear();
	m_sceneRenderer.shutdown();
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

	UniformCameraBufferBlock block;
	m_camera.getViewMatrix(&block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.position = m_camera.getPosition();
	block.qrotation = m_camera.getRotation();

	m_cameraBuffer.subData(0, sizeof(UniformCameraBufferBlock), &block);
}

void RenderAspect::taskTakeScreenshot()
{
	takeScreenshot();
}

void RenderAspect::taskLoadScene(u8* p, u32* offset)
{
	u64 address;
	::memcpy(&address, p, 8);

#if _VX_EDITOR
	auto pScene = (Editor::Scene*)address;
#else
	auto pScene = (Scene*)address;
#endif
	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Loading Scene into Render");
	m_sceneRenderer.loadScene(pScene, m_objectManager);

	auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);

	m_shadowRenderer->updateDrawCmds();

	*offset += sizeof(Scene*);
}

void RenderAspect::taskToggleRenderMode()
{
	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Toggling Rendermode");

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
	auto gpuIndex = addActorToBuffer(data->transform, data->mesh, data->material);

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
	CpuProfiler::pushMarker("clear");
	gpuProfiler->pushGpuMarker("clear");
	clearTextures();
	clearBuffers();
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

	auto instanceCount = m_sceneRenderer.getMeshInstanceCount();
	auto meshCmdBuffer = m_objectManager.getBuffer("meshCmdBuffer");
	auto meshVao = m_objectManager.getVertexArray("meshVao");
	auto meshParamBuffer = m_objectManager.getBuffer("meshParamBuffer");
	auto lightCount = m_sceneRenderer.getLightCount();

	CpuProfiler::pushMarker("shadow");
	gpuProfiler->pushGpuMarker("shadow mapping");
	m_frame.draw();
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	CpuProfiler::pushMarker("gbuffer");
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
		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));
	}
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	CpuProfiler::pushMarker("reflection");
	gpuProfiler->pushGpuMarker("reflection");
	{
		auto fbo = m_objectManager.getFramebuffer("reflectionFbo");
		auto pipeline = m_shaderManager.getPipeline("reflectionCubemap.pipe");

		vx::gl::StateManager::setViewport(0, 0, 1024, 1024);

		vx::gl::StateManager::bindPipeline(*pipeline);
		vx::gl::StateManager::bindVertexArray(*meshVao);
		vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, *meshCmdBuffer);
		vx::gl::StateManager::bindFrameBuffer(*fbo);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));
	}
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	CpuProfiler::pushMarker("voxelize");
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	gpuProfiler->pushGpuMarker("voxelize");
	voxelize(*meshVao, *meshCmdBuffer, *meshParamBuffer);
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	/*glMemoryBarrier(GL_ALL_BARRIER_BITS);
	CpuProfiler::pushMarker("pixel list");
	gpuProfiler->pushGpuMarker("pixel list");
	createConeTracePixelList();
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();*/

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	CpuProfiler::pushMarker("cone trace");
	gpuProfiler->pushGpuMarker("cone trace");
	coneTrace();
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	CpuProfiler::pushMarker("blur");
	gpuProfiler->pushGpuMarker("blur");
	blurAmbientColor();
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);
	vx::gl::StateManager::bindFrameBuffer(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	CpuProfiler::pushMarker("final");
	gpuProfiler->pushGpuMarker("final");
	m_pRenderPassFinalImage->render(1);
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();

	//voxelDebug();

	renderProfiler(gpuProfiler);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	CpuProfiler::pushMarker("swapBuffers");

	m_renderContext.swapBuffers();

	CpuProfiler::popMarker();
}

void RenderAspect::bindBuffers()
{
	m_shadowRenderer->bindBuffers();

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
	auto uniformReflectionBuffer = m_objectManager.getBuffer("UniformReflectionBuffer");

	gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	gl::BufferBindingManager::bindBaseUniform(1, lightDataBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
	gl::BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(5, pShadowTransformBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(6, pVoxelBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(7, pVoxelTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(10, renderSettingsBufferBlock->getId());
	gl::BufferBindingManager::bindBaseUniform(11, uniformReflectionBuffer->getId());

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

	//m_pColdData->m_ambientColorTexture.clearImage(0, GL_RGBA, GL_UNSIGNED_SHORT, nullptr);

	m_shadowRenderer->clearData();
}

void RenderAspect::clearBuffers()
{
	u32 count = 0;
	auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");
	auto mappedBuffer = shaderStoragePixelListCmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
	mappedBuffer->count = 0;
	mappedBuffer.unmap();
	//shaderStoragePixelListCmdBuffer->subData(0,sizeof(u32), &count);
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

void RenderAspect::voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, const vx::gl::Buffer &paramBuffer)
{
	auto voxelFB = m_objectManager.getFramebuffer("voxelFB");
	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	auto pipeline = m_shaderManager.getPipeline("voxelize.pipe");
	auto voxelizeLightPipeline = m_shaderManager.getPipeline("voxelizeLight.pipe");

	auto voxelSize = m_pColdData->m_voxelRenderer.getVoxelTextureSize();

	auto lightCount = m_sceneRenderer.getLightCount();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::bindFrameBuffer(*voxelFB);

	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);

	vx::gl::StateManager::bindPipeline(*pipeline);
	vx::gl::StateManager::bindVertexArray(vao);
	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer);
	vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Parameter_Buffer, paramBuffer);

	vx::gl::StateManager::setViewport(0, 0, voxelSize, voxelSize);
	glMultiDrawElementsIndirectCountARB(GL_TRIANGLES, GL_UNSIGNED_INT, 0, 0, 150, sizeof(vx::gl::DrawElementsIndirectCommand));

	vx::gl::StateManager::bindVertexArray(*emptyVao);
	vx::gl::StateManager::bindPipeline(*voxelizeLightPipeline);
	glDrawArrays(GL_POINTS, 0, lightCount);

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
	//auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");

	glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);

	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);
	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	auto pPipeline = m_shaderManager.getPipeline("coneTrace.pipe");

	//vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer,shaderStoragePixelListCmdBuffer->getId());
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	//glDrawArraysIndirect(GL_POINTS, 0);
	glDrawArraysInstanced(GL_POINTS, 0, m_resolution.x / 2, m_resolution.y / 2);
}

void RenderAspect::blurAmbientColor()
{
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	auto pPipeline = m_shaderManager.getPipeline("blurpass.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindFrameBuffer(m_blurFB[0]);
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	glActiveTexture(GL_TEXTURE0);
	m_pColdData->m_ambientColorTexture.bind();

	glDrawArrays(GL_POINTS, 0, 1);


	pPipeline = m_shaderManager.getPipeline("blurpass2.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	glActiveTexture(GL_TEXTURE0);
	m_pColdData->m_ambientColorBlurTexture[0].bind();

	glDrawArrays(GL_POINTS, 0, 1);

	/*

	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);



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
	//glDrawArrays(GL_POINTS, 0, 1);*/
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
	else if (key == vx::Keyboard::Key_Num9)
	{
		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::ToggleRenderMode;
		dev::g_debugRenderSettings.setShadingMode(ShadingMode::Normals);
		queueUpdateTask(task);
	}
	else if (key == vx::Keyboard::Key_F10)
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
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Queuing loading Scene into Render");
		auto pScene = (Scene*)evt.arg2.ptr;

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
		auto gpuIndex = addActorToBuffer(data->transform, data->mesh, data->material);

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

u16 RenderAspect::addActorToBuffer(const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material)
{
	vx::gl::DrawElementsIndirectCommand drawCmd;
	u32 cmdIndex = 0;
	auto gpuIndex = m_sceneRenderer.addActorToBuffer(transform, mesh, material, &drawCmd, &cmdIndex);

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