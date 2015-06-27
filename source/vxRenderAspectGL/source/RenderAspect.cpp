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
#include "vxRenderAspect/RenderAspect.h"
#include "vxRenderAspect/GpuStructs.h"
#include <vxGL/gl.h>
#include <vxGL/Debug.h>
#include <vxGL/StateManager.h>
#include "vxRenderAspect/gl/BufferBindingManager.h"
//#include "developer.h"
#include <vxGL/ProgramPipeline.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/EventsIngame.h>
//#include "ScreenshotFactory.h"
#include "vxRenderAspect/DebugRenderSettings.h"
#include <vxLib/Keyboard.h>
#include <vxEngineLib/debugPrint.h>
#include "vxRenderAspect/GpuProfiler.h"
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/EventManager.h>
#include "vxRenderAspect/Graphics/Renderer.h"
#include "vxRenderAspect/Graphics/ShadowRenderer.h"
#include "vxRenderAspect/Graphics/VoxelRenderer.h"
#include "vxRenderAspect/Graphics/Segment.h"
#include <vxLib/Window.h>
//#include "CpuProfiler.h"
#include "vxRenderAspect/Graphics/CommandListFactory.h"
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxLib/File/FileHandle.h>
#include "vxRenderAspect/Graphics/GBufferRenderer.h"

#include "vxRenderAspect/opencl/device.h"
#include "vxRenderAspect/opencl/image.h"
#include <random>

struct PlaneSimd
{
	__m128 n_d;

	static PlaneSimd create(__m128 a, __m128 b, __m128 c)
	{
		auto normal = vx::normalize3(vx::cross3(_mm_sub_ps(b,a), _mm_sub_ps(c, a)));
		auto d = vx::dot3(normal, a);

		PlaneSimd plane;
		plane.n_d = normal;
		plane.n_d.f[3] = d.f[0];

		return plane;
	}
};

struct TaskLoadScene
{
	void* ptr;
	bool editor;
};

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
	vx::gl::Texture m_aabbTexture;
	vx::gl::Buffer m_screenshotBuffer;

	vx::gl::Texture m_ambientColorTexture;
	vx::gl::Texture m_ambientColorBlurTexture[2];
	// contains index into texture array sorted by texture handle

	Font m_font;
};

RenderAspect::RenderAspect()
	: m_shadowRenderer(nullptr), 
	//m_gpuProfiler(),
	m_shaderManager(),
	m_renderContext(),
	m_camera(),
	m_fileAspect(nullptr),
	m_evtManager(nullptr),
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

void RenderAspect::createUniformBuffers(f32 znear, f32 zfar)
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
		cameraBlockStatic.orthoMatrix = vx::MatrixOrthographicRHDX(m_resolution.x, m_resolution.y, znear, zfar);

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
		auto volumeTexture = m_objectManager.getTexture("volumeTexture");

		UniformTextureBufferBlock data;
		data.u_aabbTexture = m_pColdData->m_aabbTexture.getTextureHandle();
		data.u_ambientSlice = m_pColdData->m_ambientColorTexture.getTextureHandle();
		data.u_ambientImage = m_pColdData->m_ambientColorTexture.getImageHandle(0, 0, 0);
		data.u_volumetricTexture = volumeTexture->getTextureHandle();

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

	/*const __m128 lookAt = { 0, 0, -1, 0 };
	const __m128 up = { 0, 1, 0, 0 };
	__m128 qRotation = vx::quaternionRotationRollPitchYawFromVector(vx::float4a(vx::degToRad(90.f), 0, 0, 0));
	auto viewDir = vx::quaternionRotation(lookAt, qRotation);
	auto upDir = vx::quaternionRotation(up, qRotation);
	viewDir = vx::quaternionRotation(lookAt, qRotation);

	{
		auto getTransform = [](const __m128 &position, f32 radius, UniformReflectionBufferBlock *data)
		{
			auto projectionMatrix = vx::MatrixPerspectiveFovRH(vx::degToRad(90.0f), 1.0f, 0.1f, radius);

			__m128 upAxis[6] =
			{
				// x+
				{ 0, -1, 0, 0 },
				// x-
				{ 0, -1, 0, 0 },
				// y+
				{ 0, 0, 1, 0 },
				// y-
				{ 0, 0, -1, 0 },
				// z+
				{ 0, -1, 0, 0 },
				// z-
				{ 0, -1, 0, 0 }
			};

			__m128 dir[6] =
			{
				// x+
				{ 1, 0, 0, 0 },
				// x-
				{ -1, 0, 0, 0 },
				// y+
				{ 0, 1, 0, 0 },
				// y-
				{ 0, -1, 0, 0 },
				// z+
				{ 0, 0, 1, 0 },
				// z-
				{ 0, 0, -1, 0 }
			};

			vx::mat4 viewMatrices[6];
			for (u32 i = 0; i < 6; ++i)
			{
				viewMatrices[i] = vx::MatrixLookToRH(position, dir[i], upAxis[i]);
			}

			data->positionRadius = position;
			data->positionRadius.w = radius;
			data->projectionMatrix = projectionMatrix;
			for (u32 i = 0; i < 6; ++i)
			{
				data->pvMatrix[i] = projectionMatrix * viewMatrices[i];
			}
		};

		__m128 position = { 0, 1.8f, -4, 0 };
		UniformReflectionBufferBlock block;
		getTransform(position, 2.5f, &block);

		auto texture = m_objectManager.getTexture("reflectionTexture");

		block.texture = texture->getTextureHandle();

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(UniformReflectionBufferBlock);
		desc.immutable = 1;
		desc.pData = &block;

		m_objectManager.createBuffer("UniformReflectionBuffer", desc);
	}*/
}

void RenderAspect::createTextures()
{


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

	/*{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_Cubemap;
		desc.format = vx::gl::TextureFormat::RGB16F;
		desc.size = vx::ushort3(512, 512, 6);
		desc.miplevels = 1;

		m_objectManager.createTexture("reflectionTexture", desc);

		auto texture = m_objectManager.getTexture("reflectionTexture");
		//texture->setWrapMode3D(vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER, vx::gl::TextureWrapMode::CLAMP_TO_BORDER);
		texture->makeTextureResident();

		desc.format = vx::gl::TextureFormat::DEPTH16;
		m_objectManager.createTexture("reflectionTextureDepth", desc);
	}*/

	/*{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::RGB16F;
		desc.size = vx::ushort3(m_resolution.x, m_resolution.y, 1);
		desc.miplevels = 1;

		auto sid = m_objectManager.createTexture("particleTexture", desc);
		auto tex = m_objectManager.getTexture(sid);
		tex->makeTextureResident();
	}*/

	{
		vx::gl::TextureDescription desc;
		desc.type = vx::gl::TextureType::Texture_2D;
		desc.format = vx::gl::TextureFormat::R16F;
		desc.size = vx::ushort3(m_resolution.x, m_resolution.y, 1);
		desc.miplevels = 1;

		auto sid = m_objectManager.createTexture("volumeTexture", desc);
		auto tex = m_objectManager.getTexture(sid);
		tex->makeTextureResident();
	}
}

void RenderAspect::createFrameBuffers()
{
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

	/*{
		auto colorTexture = m_objectManager.getTexture("volumeTexture");
		auto depthTexture = m_objectManager.getTexture("gbufferDepthSlice");
		auto sid = m_objectManager.createFramebuffer("volumeFbo");
		auto fbo = m_objectManager.getFramebuffer(sid);

		fbo->attachTexture(vx::gl::Attachment::Color0, *colorTexture, 0);
		fbo->attachTexture(vx::gl::Attachment::Depth, *depthTexture, 0);
	}*/
}

void RenderAspect::createColdData()
{
	m_pColdData = vx::make_unique<ColdData>();
}

void RenderAspect::provideRenderData(const EngineConfig* settings, GpuProfiler* gpuProfiler)
{
	Graphics::Renderer::provide(&m_shaderManager, &m_objectManager, settings, gpuProfiler);
}

void RenderAspect::createOpenCL()
{
	auto platformCount = cl::Platform::getPlatformCount();

	auto platforms = vx::make_unique<cl::Platform[]>(platformCount);
	cl::Platform::getPlatforms(platformCount, platforms.get());

	auto &platform = platforms[0];
	auto platformName = platform.getInfo(cl::PlatformInfo::Name);

	auto deviceCount = cl::Device::getDeviceCount(platform, cl::DeviceType::Gpu);
	cl::Device device;
	cl::Device::getDevices(platform, cl::DeviceType::Gpu, 1, &device);

	auto glcon = wglGetCurrentContext();

	cl_context_properties properties[] =
	{
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform.get(),
		0
	};

	auto error = m_context.create(properties, 1, &device);
}

bool RenderAspect::initialize(const RenderAspectDescription &desc)
{
	vx::gl::OpenGLDescription glDescription;
	glDescription.bDebugMode = desc.settings->m_renderDebug;
	glDescription.bVsync = desc.settings->m_vsync;
	glDescription.farZ = desc.settings->m_zFar;
	glDescription.fovRad = vx::degToRad(desc.settings->m_fov);
	glDescription.hwnd = desc.window->getHwnd();
	glDescription.majVersion = 4;
	glDescription.minVersion = 5;
	glDescription.nearZ = desc.settings->m_zNear;
	glDescription.resolution = desc.settings->m_resolution;

	vx::gl::ContextDescription contextDesc;
	contextDesc.glParams = glDescription;
	contextDesc.hInstance = desc.window->getHinstance();
	contextDesc.windowClass = desc.window->getClassName();

	m_fileAspect = desc.fileAspect;
	m_evtManager = desc.evtManager;

	//m_gpuProfiler = vx::make_unique<GpuProfiler>();

	if (!initializeCommon(contextDesc, desc.settings))
		return false;

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	m_shaderManager.addParameter("maxShadowLights", desc.settings->m_rendererSettings.m_shadowSettings.m_maxShadowCastingLights);

	auto result = initializeImpl(desc.dataDir, desc.settings, desc.pAllocator);
	if (!result)
		return false;

	auto gbufferRenderer = vx::make_unique<Graphics::GBufferRenderer>();
	gbufferRenderer->initialize(&m_allocator);
	m_renderer.push_back(std::move(gbufferRenderer));

	if (desc.settings->m_rendererSettings.m_shadowMode != 0)
	{
		auto shadowRenderer = vx::make_unique<Graphics::ShadowRenderer>();
		shadowRenderer->initialize(&m_allocator);

		m_shadowRenderer = shadowRenderer.get();
		m_renderer.push_back(std::move(shadowRenderer));
	}

	// create gbuffer

	if (desc.settings->m_rendererSettings.m_voxelGIMode != 0)
	{
		auto voxelRenderer = vx::make_unique<Graphics::VoxelRenderer >();
		voxelRenderer->initialize(&m_allocator);
		m_renderer.push_back(std::move(voxelRenderer));
	}

	createOpenCL();

	createFrame();

	bindBuffers();

	return result;
}

bool RenderAspect::initializeCommon(const vx::gl::ContextDescription &contextDesc, const EngineConfig* settings)
{
	createColdData();

	if (!m_renderContext.initialize(contextDesc))
		return false;

	provideRenderData(settings, nullptr);

	return true;
}

bool RenderAspect::initializeImpl(const std::string &dataDir, const EngineConfig* engineConfig, vx::StackAllocator *pAllocator)
{
	m_resolution = engineConfig->m_resolution;

	m_allocator = vx::StackAllocator(pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);

	if (engineConfig->m_renderDebug)
	{
		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(::debugCallback);
		vx::gl::Debug::enableCallback(true);
	}

	vx::gl::StateManager::disable(vx::gl::Capabilities::Framebuffer_sRGB);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	m_objectManager.initialize(50, 20, 20, 20, &m_allocator);

	m_camera.setPosition(0, 2.5f, 15);

	if (!m_shaderManager.initialize(dataDir, &m_allocator, false))
	{
		puts("Error initializing Shadermanager");
		return false;
	}

	m_shaderManager.loadPipeline(vx::FileHandle("draw_final_image.pipe"), "draw_final_image.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("drawFinalImageAlbedo.pipe"), "drawFinalImageAlbedo.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("drawFinalImageNormals.pipe"), "drawFinalImageNormals.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("voxelize.pipe"), "voxelize.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("voxelizeLight.pipe"), "voxelizeLight.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("coneTrace.pipe"), "coneTrace.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("resetLightCmdBuffer.pipe"), "resetLightCmdBuffer.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("createLightCmdBuffer.pipe"), "createLightCmdBuffer.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("shadow.pipe"), "shadow.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("blurpass.pipe"), "blurpass.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("blurpass2.pipe"), "blurpass2.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("text.pipe"), "text.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("screenquad.pipe"), "screenquad.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("volume.pipe"), "volume.pipe", &m_allocator);

	const auto doubleBufferSizeInBytes = 5 KBYTE;
	m_doubleBuffer = DoubleBufferRaw(&m_allocator, doubleBufferSizeInBytes);

	createTextures();
	createFrameBuffers();

	createUniformBuffers(engineConfig->m_zNear, engineConfig->m_zFar);
	createBuffers();

	m_sceneRenderer.initialize(10, &m_objectManager, pAllocator);

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
	auto pipeline = m_shaderManager.getPipeline("drawFinalImageAlbedo.pipe");
	m_renderpassFinalImageAlbedo.initialize(*emptyVao, *pipeline, m_resolution);
	pipeline = m_shaderManager.getPipeline("drawFinalImageNormals.pipe");
	m_renderpassFinalImageNormals.initialize(*emptyVao, *pipeline, m_resolution);

	m_pRenderPassFinalImage = &m_renderpassFinalImageFullShading;

	return true;
}

bool RenderAspect::initializeProfiler()
{
	/*auto fontHandle = glGetTextureHandleARB(m_pColdData->m_font.getTextureEntry().getTextureId());
	auto profiler = m_gpuProfiler.get();
	if (!m_sceneRenderer.initializeProfiler(m_pColdData->m_font, fontHandle, m_resolution, m_shaderManager, profiler, &m_allocator))
	{
		return false;
	}*/

	return true;
}

void RenderAspect::createFrame()
{
	m_frame.clear();

	for (auto &it : m_renderer)
	{
		Graphics::CommandList cmdList;
		cmdList.initialize();
		it->getCommandList(&cmdList);

		m_frame.pushCommandList(std::move(cmdList));
	}

	//m_shadowRenderer->getCommandList(&cmdList);
}

void RenderAspect::shutdown(void* hwnd)
{
	for (auto &it : m_renderer)
	{
		it->shutdown();
	}

	//m_gpuProfiler.reset(nullptr);
	m_tasks.clear();
	m_sceneRenderer.shutdown();
	m_objectManager.shutdown();
	m_pColdData.reset(nullptr);
	m_renderContext.shutdown((HWND)hwnd);
}

void RenderAspect::makeCurrent(bool b)
{
	m_renderContext.makeCurrent(b);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTask &task)
{
	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_tasks.push_back(task);
}

void RenderAspect::queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize)
{
	vx::lock_guard<vx::mutex> lck(m_updateMutex);

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

	vx::lock_guard<vx::mutex> lck(m_updateMutex);
	m_updateCameraData = data;

	m_tasks.push_back(task);
}

void RenderAspect::update()
{
	vx::lock_guard<vx::mutex> lck(m_updateMutex);
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

void RenderAspect::updateProfiler(f32 dt)
{
	//m_gpuProfiler->update(dt);
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
	TaskLoadScene* data = (TaskLoadScene*)p;

	vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Loading Scene into Render");
	m_sceneRenderer.loadScene(data->ptr, m_objectManager, m_fileAspect, data->editor);

	auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);

	if (m_shadowRenderer)
		m_shadowRenderer->updateDrawCmds();

	*offset += sizeof(TaskLoadScene);
}

void RenderAspect::taskToggleRenderMode()
{
	/*vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Toggling Rendermode");

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
	}*/
}

void RenderAspect::taskCreateActorGpuIndex(u8* p, u32* offset)
{
	std::size_t* address = (std::size_t*)p;

	CreateActorData* data = (CreateActorData*)(*address);
	auto gpuIndex = addActorToBuffer(data->getTransform(), data->getMeshSid(), data->getMaterialSid());
	data->setGpu(gpuIndex);

	vx::Event e;
	e.arg1.ptr = data;
	e.code = (u32)IngameEvent::Created_Actor_GPU;
	e.type = vx::EventType::Ingame_Event;

	m_evtManager->addEvent(e);

	*offset += sizeof(std::size_t);
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

void RenderAspect::render()
{
	//m_gpuProfiler->frame();

	//CpuProfiler::pushMarker("clear");
	//m_gpuProfiler->pushGpuMarker("clear");
	clearTextures();
	clearBuffers();
	//m_gpuProfiler->popGpuMarker();
//	CpuProfiler::popMarker();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

	auto instanceCount = m_sceneRenderer.getMeshInstanceCount();
	auto meshCmdBuffer = m_objectManager.getBuffer("meshCmdBuffer");
	auto meshVao = m_objectManager.getVertexArray("meshVao");
	auto meshParamBuffer = m_objectManager.getBuffer("meshParamBuffer");
	auto lightCount = m_sceneRenderer.getLightCount();

	//CpuProfiler::pushMarker("frame");
	//m_gpuProfiler->pushGpuMarker("frame");
	m_frame.draw();
	//m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	//CpuProfiler::pushMarker("voxelize");
	//m_gpuProfiler->pushGpuMarker("voxelize");
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	voxelize(*meshVao, *meshCmdBuffer, *meshParamBuffer);
	//m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	//CpuProfiler::pushMarker("cone trace");
	//m_gpuProfiler->pushGpuMarker("cone trace");
	coneTrace();
	//m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	//__m128 angles = { 0, vx::degToRad(-90), 0, 0 };
	//auto matrix = vx::MatrixRotationRollPitchYaw(angles);

	/*CpuProfiler::pushMarker("volumetric light");
	gpuProfiler->pushGpuMarker("volumetric light");
	{
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);

		glDepthMask(GL_FALSE);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		auto pPipeline = m_shaderManager.getPipeline("volume.pipe");
		auto fbo = m_objectManager.getFramebuffer("volumeFbo");
		auto vao = m_objectManager.getVertexArray("emptyVao");

		vx::gl::StateManager::bindFrameBuffer(fbo->getId());
		glClear(GL_COLOR_BUFFER_BIT);

		vx::gl::StateManager::bindVertexArray(vao->getId());
		vx::gl::StateManager::bindPipeline(pPipeline->getId());

		glDrawArrays(GL_POINTS, 0, 1);


		glDepthMask(GL_TRUE);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	}
	gpuProfiler->popGpuMarker();
	CpuProfiler::popMarker();*/

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//CpuProfiler::pushMarker("blur");
	//m_gpuProfiler->pushGpuMarker("blur");
	blurAmbientColor();
	//m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);
	vx::gl::StateManager::bindFrameBuffer(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//CpuProfiler::pushMarker("final");
	//m_gpuProfiler->pushGpuMarker("final");
	m_pRenderPassFinalImage->render(1);
	//m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	//voxelDebug();

	renderProfiler();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//CpuProfiler::pushMarker("swapBuffers");

	m_renderContext.swapBuffers();

//	CpuProfiler::popMarker();
}

void RenderAspect::bindBuffers()
{
	for (auto &it : m_renderer)
	{
		it->bindBuffers();
	}

	auto pShadowTransformBuffer = m_objectManager.getBuffer("ShadowTransformBuffer");
	auto pTextureBuffer = m_objectManager.getBuffer("TextureBuffer");
	auto pUniformTextureBuffer = m_objectManager.getBuffer("UniformTextureBuffer");
	auto pCameraBufferStatic = m_objectManager.getBuffer("CameraBufferStatic");
	auto shaderStoragePixelListCmdBuffer = m_objectManager.getBuffer("ShaderStoragePixelListCmdBuffer");
	auto shaderStorageConetracePixelListBuffer = m_objectManager.getBuffer("ShaderStorageConetracePixelListBuffer");
	auto renderSettingsBufferBlock = m_objectManager.getBuffer("RenderSettingsBufferBlock");

	auto transformBuffer = m_objectManager.getBuffer("transformBuffer");
	auto materialBlockBuffer = m_objectManager.getBuffer("materialBlockBuffer");
	auto lightDataBuffer = m_objectManager.getBuffer("lightDataBuffer");
	auto meshCmdIndexBuffer = m_objectManager.getBuffer("meshCmdIndexBuffer");
//	auto uniformReflectionBuffer = m_objectManager.getBuffer("UniformReflectionBuffer");

	gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	gl::BufferBindingManager::bindBaseUniform(1, lightDataBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
	gl::BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseUniform(5, pShadowTransformBuffer->getId());

	gl::BufferBindingManager::bindBaseUniform(10, renderSettingsBufferBlock->getId());

	gl::BufferBindingManager::bindBaseShaderStorage(0, transformBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(1, materialBlockBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(2, pTextureBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(3, shaderStorageConetracePixelListBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(4, shaderStoragePixelListCmdBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(6, meshCmdIndexBuffer->getId());
}

void RenderAspect::clearTextures()
{
	for (auto &it : m_renderer)
	{
		it->clearData();
	}
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

void RenderAspect::voxelize(const vx::gl::VertexArray &vao, const vx::gl::Buffer &cmdBuffer, const vx::gl::Buffer &paramBuffer)
{
	auto voxelFB = m_objectManager.getFramebuffer("voxelFB");
	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	auto pipeline = m_shaderManager.getPipeline("voxelize.pipe");
	auto voxelizeLightPipeline = m_shaderManager.getPipeline("voxelizeLight.pipe");

	auto voxelSize = 128;

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
	//auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	//m_pColdData->m_voxelRenderer.debug(*emptyVao, m_resolution);
}

void RenderAspect::createConeTracePixelList()
{
	/*glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	auto emptyVao = m_objectManager.getVertexArray("emptyVao");

	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	auto pPipeline = m_shaderManager.getPipeline("createConeTraceList.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	glDrawArrays(GL_POINTS, 0, 1);*/
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

void RenderAspect::renderProfiler()
{
	/*vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_gpuProfiler->render();

	CpuProfiler::render();

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);*/
}

void RenderAspect::keyPressed(u16 key)
{
	/*if (key == vx::Keyboard::Key_Num0)
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
	}*/
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

	//ScreenshotFactory::writeScreenshotToFile(m_resolution, pScreenshotData);
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

		TaskLoadScene data;
		data.ptr = pScene;
		data.editor = false;

		queueUpdateTask(task, (u8*)&data, sizeof(TaskLoadScene));
	}break;
	case vx::FileEvent::EditorScene_Loaded:
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Queuing loading Scene into Render");
		auto pScene = (Scene*)evt.arg2.ptr;

		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::LoadScene;

		TaskLoadScene data;
		data.ptr = pScene;
		data.editor = true;

		queueUpdateTask(task, (u8*)&data, sizeof(TaskLoadScene));
	}break;
	default:
		break;
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
	auto gpuIndex = m_sceneRenderer.addActorToBuffer(transform, mesh, material, &drawCmd, &cmdIndex, m_fileAspect);

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

void RenderAspect::getTotalVRam(u32* totalVram) const
{
	glGetIntegerv(GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, (s32*)totalVram);
}

void RenderAspect::getTotalAvailableVRam(u32* totalAvailableVram) const
{
	glGetIntegerv(GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, (s32*)totalAvailableVram);
}

void RenderAspect::getAvailableVRam(u32* usedVram) const
{
	glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, (s32*)usedVram);
}