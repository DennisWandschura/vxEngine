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
#include <vxGL/gl.h>
#include <vxGL/Debug.h>
#include <vxGL/StateManager.h>
#include "gl/BufferBindingManager.h"
//#include "developer.h"
#include <vxGL/ProgramPipeline.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/EventsIngame.h>
//#include "ScreenshotFactory.h"
#include "DebugRenderSettings.h"
#include <vxLib/Keyboard.h>
#include <vxEngineLib/debugPrint.h>
#include "GpuProfiler.h"
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/EventManager.h>
#include "Graphics/Renderer.h"
#include "Graphics/ShadowRenderer.h"
#include "Graphics/VoxelRenderer.h"
#include "Graphics/Segment.h"
#include <vxLib/Window.h>
//#include "CpuProfiler.h"
#include "Graphics/CommandListFactory.h"
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxLib/File/FileHandle.h>
#include "Graphics/GBufferRenderer.h"
#include <vxEngineLib/Scene.h>
#include "Frustum.h"
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Graphics/TextureFactory.h>
#include <vxEngineLib/Graphics/Texture.h>
#include <vxEngineLib/Material.h>
#include <vxGL/VertexArray.h>
#include "Graphics/VolumetricLightRenderer.h"
#include <vxEngineLib/CreateDynamicMeshData.h>

#include "opencl/device.h"
#include "opencl/image.h"
#include <random>

struct PlaneSimd
{
	vx::float4a n_d;

	static PlaneSimd create(__m128 a, __m128 b, __m128 c)
	{
		auto normal = vx::normalize3(vx::cross3(_mm_sub_ps(b, a), _mm_sub_ps(c, a)));
		vx::float4a d = vx::dot3(normal, a);

		PlaneSimd plane;
		plane.n_d = normal;
		plane.n_d.w = d.x;

		return plane;
	}
};

struct TaskLoadScene
{
	void* ptr;
	bool editor;
};

RenderAspect* g_renderAspect{ nullptr };

namespace RenderAspectCpp
{
	void __stdcall debugCallback()
	{
		std::abort();
	}

	f32 distancePointAABB(const vx::float3 &p, const AABB &b)
	{
		f32 sqDist = 0.0f;
		for (u32 i = 0;i < 3; ++i)
		{
			auto v = p[i];

			if (v < b.min[i])
			{
				sqDist += (b.min[i] - v) * (b.min[i] - v);
			}

			if (v > b.max[i])
			{
				sqDist += (v - b.max[i]) * (v - b.max[i]);
			}
		}

		return sqDist;
	}

	f32 distancePointAABB2(const vx::float3 &p, const AABB &b)
	{
		auto vp = vx::loadFloat3(p);
		auto bmin = vx::loadFloat3(b.min);
		auto bmax = vx::loadFloat3(b.max);

		auto vmin = _mm_min_ps(vp, bmin);
		auto vmax = _mm_max_ps(vp, bmax);

		auto vdistMin = _mm_sub_ps(bmin, vmin);
		auto vdistMax = _mm_sub_ps(vmax, bmax);

		auto d0 = vx::dot3(vdistMin, vdistMin);
		auto d1 = vx::dot3(vdistMax, vdistMax);
		auto dist = _mm_add_ps(d0, d1);

		f32 result;
		_mm_store_ss(&result, dist);
		return result;
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
	m_gpuProfiler(),
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
		cameraBlockStatic.invProjectionMatrix = vx::MatrixInverse(m_projectionMatrix);
		cameraBlockStatic.projectionMatrix = m_projectionMatrix;
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
		Gpu::UniformTextureBufferBlock data;
		data.u_aabbTexture = m_pColdData->m_aabbTexture.getTextureHandle();
		data.u_ambientSlice = m_pColdData->m_ambientColorTexture.getTextureHandle();
		data.u_ambientImage = m_pColdData->m_ambientColorTexture.getImageHandle(0, 0, 0);
		data.u_volumetricTexture = 0;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(Gpu::UniformTextureBufferBlock);
		desc.immutable = 1;
		desc.pData = &data;
		desc.flags = vx::gl::BufferStorageFlags::Write;

		m_objectManager.createBuffer("UniformTextureBuffer", desc);
	}

	{
		Gpu::RenderSettingsBlock data;
		data.resolution = m_resolution;

		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		desc.size = sizeof(Gpu::RenderSettingsBlock);
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
		desc.size = vx::ushort3(m_resolution.x / 4, m_resolution.y / 4, 1);
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

RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &desc)
{
	//m_projectionMatrix = MatrixPerspectiveFovRH(params.fovRad, screenAspect, params.nearZ, params.farZ);
	auto screenAspect = (f32)desc.settings->m_resolution.x / (f32)desc.settings->m_resolution.y;
	auto fovRad = vx::degToRad(desc.settings->m_fov);
	m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(fovRad, screenAspect, desc.settings->m_zNear, desc.settings->m_zFar);

	vx::gl::OpenGLDescription glDescription;
	glDescription.bDebugMode = desc.settings->m_renderDebug;
	glDescription.bVsync = desc.settings->m_vsync;
	glDescription.hwnd = desc.window->getHwnd();
	glDescription.majVersion = 4;
	glDescription.minVersion = 5;
	glDescription.resolution = desc.settings->m_resolution;

	vx::gl::ContextDescription contextDesc;
	contextDesc.glParams = glDescription;
	contextDesc.hInstance = desc.window->getHinstance();
	contextDesc.windowClass = desc.window->getClassName();

	m_fileAspect = desc.fileAspect;
	m_evtManager = desc.evtManager;

	m_gpuProfiler = vx::make_unique<GpuProfiler>();
	m_pColdData = vx::make_unique<ColdData>();

	if (!m_renderContext.initialize(contextDesc))
		return RenderAspectInitializeError::ERROR_CONTEXT;

	Graphics::Renderer::provide(&m_shaderManager, &m_objectManager, desc.settings, m_gpuProfiler.get());

	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

	m_shaderManager.addParameter("maxShadowLights", desc.settings->m_rendererSettings.m_shadowSettings.m_maxShadowCastingLights);

	m_resolution = desc.settings->m_resolution;

	m_allocator = vx::StackAllocator(desc.pAllocator->allocate(5 MBYTE, 64), 5 MBYTE);
	m_scratchAllocator = vx::StackAllocator(desc.pAllocator->allocate(10 MBYTE, 64), 10 MBYTE);

	if (desc.settings->m_renderDebug)
	{
		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(RenderAspectCpp::debugCallback);
		vx::gl::Debug::enableCallback(true);
	}

	vx::gl::StateManager::disable(vx::gl::Capabilities::Framebuffer_sRGB);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
	vx::gl::StateManager::setClearColor(0, 0, 0, 1);
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

	m_objectManager.initialize(50, 20, 20, 20, &m_allocator);

	m_camera.setPosition(0, 2.5f, 15);

	if (!m_shaderManager.initialize(desc.dataDir, &m_allocator, false))
	{
		puts("Error initializing Shadermanager");
		return RenderAspectInitializeError::ERROR_SHADER;
	}

	m_shaderManager.addParameter("maxActiveLights", desc.settings->m_rendererSettings.m_maxActiveLights);
	m_shaderManager.setDefine("FULL_SHADING");

	auto shaderIncludeDir = desc.dataDir + "shaders/include/";
	m_shaderManager.addIncludeFile((shaderIncludeDir + "structs.glsl").c_str(), "structs.glsl");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "math.glsl").c_str(), "math.glsl");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "buffers.glsl").c_str(), "buffers.glsl");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "uniform_buffers.glsl").c_str(), "uniform_buffers.glsl");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "mesh.glsl").c_str(), "mesh.glsl");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "common.h").c_str(), "common.h");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformCameraBuffer.h").c_str(), "UniformCameraBuffer.h");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformShadowTextureBuffer.h").c_str(), "UniformShadowTextureBuffer.h");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformShadowTransformBuffer.h").c_str(), "UniformShadowTransformBuffer.h");
	m_shaderManager.addIncludeFile((shaderIncludeDir + "UniformCameraBufferStatic.h").c_str(), "UniformCameraBufferStatic.h");

	m_shaderManager.loadPipeline(vx::FileHandle("draw_final_image.pipe"), "draw_final_image.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("drawFinalImageAlbedo.pipe"), "drawFinalImageAlbedo.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("drawFinalImageNormals.pipe"), "drawFinalImageNormals.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("coneTrace.pipe"), "coneTrace.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("blurpass.pipe"), "blurpass.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("blurpassNew.pipe"), "blurpassNew.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("blurpass2.pipe"), "blurpass2.pipe", &m_allocator);
	m_shaderManager.loadPipeline(vx::FileHandle("voxel_debug.pipe"), "voxel_debug.pipe", &m_allocator);

	m_shaderManager.loadPipeline(vx::FileHandle("screenquad.pipe"), "screenquad.pipe", &m_allocator);

	const auto doubleBufferSizeInBytes = 5 KBYTE;
	m_doubleBuffer = DoubleBufferRaw(&m_allocator, doubleBufferSizeInBytes);

	createTextures();
	createFrameBuffers();

	createUniformBuffers(desc.settings->m_zNear, desc.settings->m_zFar);
	createBuffers();

	//m_sceneRenderer.initialize(10, &m_objectManager, desc.pAllocator);
	m_materialManager.initialize(vx::uint3(1024, 1024, 128), desc.settings->m_rendererSettings.m_maxMeshInstances, &m_objectManager);
	m_meshManager.initialize(desc.settings->m_rendererSettings.m_maxMeshInstances, 0xffff+1, 0xffff+1, &m_objectManager);

	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	m_renderpassFinalImageFullShading.initialize(*emptyVao, *m_shaderManager.getPipeline("draw_final_image.pipe"), m_resolution);
	auto pipeline = m_shaderManager.getPipeline("drawFinalImageAlbedo.pipe");
	m_renderpassFinalImageAlbedo.initialize(*emptyVao, *pipeline, m_resolution);
	pipeline = m_shaderManager.getPipeline("drawFinalImageNormals.pipe");
	m_renderpassFinalImageNormals.initialize(*emptyVao, *pipeline, m_resolution);

	m_pRenderPassFinalImage = &m_renderpassFinalImageFullShading;

	auto lightRenderer = vx::make_unique<Graphics::LightRenderer>();
	lightRenderer->initialize(&m_allocator, nullptr);
	m_lightRenderer = lightRenderer.get();
	m_renderer.push_back(std::move(lightRenderer));

	auto gbufferRenderer = vx::make_unique<Graphics::GBufferRenderer>();
	gbufferRenderer->initialize(&m_allocator, nullptr);
	m_renderer.push_back(std::move(gbufferRenderer));

	if (desc.settings->m_rendererSettings.m_shadowMode != 0)
	{
		auto shadowRenderer = vx::make_unique<Graphics::ShadowRenderer>();
		shadowRenderer->initialize(&m_allocator, nullptr);

		m_shadowRenderer = shadowRenderer.get();
		m_renderer.push_back(std::move(shadowRenderer));
	}

	// create gbuffer

	if (desc.settings->m_rendererSettings.m_voxelGIMode != 0)
	{
		auto voxelRenderer = vx::make_unique<Graphics::VoxelRenderer >();
		voxelRenderer->initialize(&m_allocator, nullptr);
		m_renderer.push_back(std::move(voxelRenderer));
	}

	auto volumetricRenderer = vx::make_unique<Graphics::VolumetricLightRenderer>();
	volumetricRenderer->initialize(&m_allocator, nullptr);
	m_renderer.push_back(std::move(volumetricRenderer));

	createOpenCL();

	createFrame();

	bindBuffers();

	printf("Used Memory (Buffers): %.2f MB\n", m_objectManager.getUsedMemoryBuffer() / 1024.f / 1024.f);

	f32 px = desc.settings->m_resolution.x / 2 - 100;
	f32 py = desc.settings->m_resolution.y * 0.45f;
	m_pstateProfiler.initialize(vx::float2(px, py));

	return RenderAspectInitializeError::OK;
}
bool RenderAspect::initializeProfiler()
{
	u32 textureIndex = 0;
	u64 fontHandle = 0;
	{
		std::string dataDir = "../data/";
		auto file = (dataDir + "textures/verdana.png");
		auto sid = vx::make_sid("verdana.png");

		Graphics::Texture texture;
		Graphics::TextureFactory::createPngFromFile(file.c_str(), true, &texture, &m_allocator, &m_scratchAllocator);

		auto b = m_materialManager.getTextureIndex(sid, texture, &textureIndex);
		VX_ASSERT(b);
		auto texId = m_materialManager.getTextureId(sid);

		auto dim = texture.getFace(0).getDimension();

		FontAtlas fontAtlas;
		if (!fontAtlas.loadFromFile((dataDir + "fonts/meta/VerdanaRegular.sdff").c_str()))
			return false;

		VX_ASSERT(dim.x == dim.y);
		m_pColdData->m_font = Font(textureIndex, dim.x,std::move(fontAtlas));

		fontHandle = glGetTextureHandleARB(texId);
	}

	Graphics::TextRendererDesc desc;
	desc.font = &m_pColdData->m_font;
	desc.maxCharacters = 512;
	desc.textureIndex = textureIndex;
	desc.allocator = &m_allocator;

	m_textRenderer = vx::make_unique<Graphics::TextRenderer>();
	m_textRenderer->initialize(&m_allocator, &desc);

	m_textCmdList.initialize();
	m_textRenderer->getCommandList(&m_textCmdList);

	auto xpos = s32(m_resolution.x / 2) - 20;
	auto ypos = s32(m_resolution.y / 2) - 20;

	m_gpuProfiler->initialize(vx::float2(-xpos, ypos), m_textRenderer.get(), &m_allocator);

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

		if (cmdList.size() != 0)
		{
			m_frame.pushCommandList(std::move(cmdList));
		}
	}

	//m_shadowRenderer->getCommandList(&cmdList);
}

void RenderAspect::shutdown(void* hwnd)
{
	for (auto &it : m_renderer)
	{
		it->shutdown();
	}

	m_textRenderer.reset(nullptr);
	m_gpuProfiler.reset(nullptr);
	m_tasks.clear();
	//m_sceneRenderer.shutdown();
	m_materialManager.shutdown();
	m_meshManager.shutdown();
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
	processTasks();

	m_lightRenderer->cullLights(m_camera);

	if (m_shadowRenderer)
	{
		vx::mat4 viewMatrix;
		m_camera.getViewMatrix(&viewMatrix);

		auto pvMatrix = m_projectionMatrix * viewMatrix;
		Frustum frustum;
		frustum.update(pvMatrix);

		m_shadowRenderer->cullLights(frustum, m_camera);
	}
}

void RenderAspect::processTasks()
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
		case RenderUpdateTask::Type::UpdateText:
			taskUpdateText(p, &offset);
			break;
		case RenderUpdateTask::Type::CreateActorGpuIndex:
			taskCreateActorGpuIndex(p, &offset);
			break;
		case RenderUpdateTask::Type::AddStaticMeshInstance:
			taskAddStaticMeshInstance(p, &offset);
			break;
		case RenderUpdateTask::Type::AddDynamicMeshInstance:
			taskAddDynamicMeshInstance(p, &offset);
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
			VX_ASSERT(false);
			break;
		}
	}
	m_tasks.clear();

	VX_ASSERT(offset == backBufferSize);
}

void RenderAspect::updateProfiler(f32 dt)
{
	m_pstateProfiler.update(m_textRenderer.get());

	m_gpuProfiler->update();
	m_textRenderer->update();
}

void RenderAspect::taskUpdateCamera()
{
	m_camera.setPosition(m_updateCameraData.position);
	m_camera.setRotation(m_updateCameraData.quaternionRotation);

	//auto projectionMatrix = m_renderContext.getProjectionMatrix();

	UniformCameraBufferBlock block;
	m_camera.getViewMatrix(&block.viewMatrix);
	block.pvMatrix = m_projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.position = m_camera.getPosition();
	block.qrotation = m_camera.getRotation();

	m_cameraBuffer.subData(0, sizeof(UniformCameraBufferBlock), &block);
}

void RenderAspect::taskUpdateText(u8* p, u32* offset)
{
	auto data = (RenderUpdateTextData*)p;

	m_textRenderer->pushEntry(data->tex, data->size, data->position, data->color);

	*offset += sizeof(RenderUpdateTextData);
}

void RenderAspect::taskTakeScreenshot()
{
	takeScreenshot();
}

void RenderAspect::taskLoadScene(u8* p, u32* offset)
{
	TaskLoadScene* data = (TaskLoadScene*)p;
	auto scene = (Scene*)data->ptr;

	//vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Render, "Loading Scene into Render");
	//m_sceneRenderer.loadScene(scene, m_objectManager, m_fileAspect);

	/*auto instances = scene->getMeshInstances();
	auto instanceCount = scene->getMeshInstanceCount();
	for (u32 i = 0; i < instanceCount; ++i)
	{
		auto &instance = instances[i];
		u32 materialIndex = 0;
		auto b = m_materialManager.getMaterialIndex(*instance.getMaterial().get(), m_fileAspect, &materialIndex);
		VX_ASSERT(b);

		auto gpuIndex = m_meshManager.addMeshInstance(instance, materialIndex, m_fileAspect);
	}*/


	//auto count = m_sceneRenderer.getMeshInstanceCount();

	if (m_shadowRenderer)
		m_shadowRenderer->updateDrawCmds();

	auto lightCount = scene->getLightCount();
	auto lights = scene->getLights();

	m_lightRenderer->setLights(lights, lightCount);

	if (m_shadowRenderer)
	{
		m_shadowRenderer->setLights(lights, lightCount);
	}

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
	auto gpuIndex = addActorToBuffer(data->getActorSid(), data->getTransform(), data->getMeshSid(), data->getMaterialSid());
	data->setGpu(gpuIndex);

	vx::Event e;
	e.arg1.ptr = data;
	e.code = (u32)IngameEvent::Gpu_AddedActor;
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
		m_meshManager.updateTransform(transforms[i], indices[i]);
	}

	*offset += sizeof(RenderUpdateDataTransforms) + (sizeof(vx::TransformGpu) + sizeof(u32)) * count;
}

void RenderAspect::taskAddStaticMeshInstance(u8* p, u32* offset)
{
	RenderUpdateTaskAddStaticMeshData* data = (RenderUpdateTaskAddStaticMeshData*)p;

	auto &instance = *data->instance;

	u32 materialIndex = 0;
	m_materialManager.getMaterialIndex(data->materialSid, m_fileAspect, &materialIndex);

	u32 gpuIndex = m_meshManager.addMeshInstance(instance, materialIndex, m_fileAspect);

	*offset += sizeof(RenderUpdateTaskAddStaticMeshData);
}

void RenderAspect::taskAddDynamicMeshInstance(u8* p, u32* offset)
{
	std::size_t* address = (std::size_t*)p;

	CreateDynamicMeshData* data = (CreateDynamicMeshData*)(*address);
	auto &instance = *data->m_meshInstance;

	u32 materialIndex = 0;
	m_materialManager.getMaterialIndex(data->m_materialSid, m_fileAspect, &materialIndex);

	u32 gpuIndex = m_meshManager.addMeshInstance(instance, materialIndex, m_fileAspect);

	data->m_gpuIndex = gpuIndex;
	data->increment();

	vx::Event e;
	e.code = (u32)IngameEvent::Gpu_AddedDynamicMesh;
	e.type = vx::EventType::Ingame_Event;
	e.arg1.ptr = data;

	m_evtManager->addEvent(e);

	*offset += sizeof(std::size_t);
}

void RenderAspect::submitCommands()
{
	m_gpuProfiler->frame();
	m_gpuProfiler->pushGpuMarker("frame");

	//CpuProfiler::pushMarker("clear");
	m_gpuProfiler->pushGpuMarker("clear");
	clearTextures();
	clearBuffers();
	m_gpuProfiler->popGpuMarker();
	//	CpuProfiler::popMarker();

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);

	//CpuProfiler::pushMarker("frame");
	m_gpuProfiler->pushGpuMarker("commands");
	m_frame.draw();
	m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

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

	vx::gl::StateManager::setColorMask(1, 1, 1, 1);
	vx::gl::StateManager::setDepthMask(1);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	//CpuProfiler::pushMarker("cone trace");
	m_gpuProfiler->pushGpuMarker("cone trace");
	coneTrace();
	m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	//CpuProfiler::pushMarker("blur");
	m_gpuProfiler->pushGpuMarker("blur");
	blurAmbientColor();
	m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);
	vx::gl::StateManager::bindFrameBuffer(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	//CpuProfiler::pushMarker("final");
	m_gpuProfiler->pushGpuMarker("final");
	m_pRenderPassFinalImage->render(1);
	m_gpuProfiler->popGpuMarker();
	//CpuProfiler::popMarker();

	//voxelDebug();

	/*{
		vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);
		vx::gl::StateManager::bindFrameBuffer(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto pPipeline = m_shaderManager.getPipeline("voxel_debug.pipe");
		auto emptyVao = m_objectManager.getVertexArray("emptyVao");

		vx::gl::StateManager::disable(vx::gl::Capabilities::Cull_Face);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindVertexArray(emptyVao->getId());
		vx::gl::StateManager::bindPipeline(pPipeline->getId());

		glDrawArraysInstanced(GL_POINTS, 0, 128, 128*128);

		vx::gl::StateManager::enable(vx::gl::Capabilities::Cull_Face);
		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	}*/

	renderProfiler();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_gpuProfiler->pushGpuMarker("wait");
	m_fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	m_gpuProfiler->popGpuMarker();
}

void RenderAspect::endFrame()
{
	auto result = glClientWaitSync((GLsync)m_fence, 0, 0);
	m_gpuProfiler->popGpuMarker();
	//glFinish();
	m_gpuProfiler->pushGpuMarker("swap buffer");

	m_renderContext.swapBuffers();

	m_gpuProfiler->popGpuMarker();
}

void RenderAspect::bindBuffers()
{
	for (auto &it : m_renderer)
	{
		it->bindBuffers();
	}

	auto pUniformTextureBuffer = m_objectManager.getBuffer("UniformTextureBuffer");
	auto pCameraBufferStatic = m_objectManager.getBuffer("CameraBufferStatic");
	auto renderSettingsBufferBlock = m_objectManager.getBuffer("RenderSettingsBufferBlock");

	auto transformBuffer = m_objectManager.getBuffer("transformBuffer");
	auto materialBlockBuffer = m_objectManager.getBuffer("materialBlockBuffer");

	//	auto uniformReflectionBuffer = m_objectManager.getBuffer("UniformReflectionBuffer");

	gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
	gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());
	gl::BufferBindingManager::bindBaseUniform(3, pUniformTextureBuffer->getId());

	gl::BufferBindingManager::bindBaseUniform(10, renderSettingsBufferBlock->getId());

	gl::BufferBindingManager::bindBaseShaderStorage(0, transformBuffer->getId());
	gl::BufferBindingManager::bindBaseShaderStorage(1, materialBlockBuffer->getId());
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

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);
	glClear(GL_COLOR_BUFFER_BIT);

	vx::gl::StateManager::setViewport(0, 0, m_resolution.x / 4, m_resolution.y / 4);

	auto pPipeline = m_shaderManager.getPipeline("coneTrace.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::blurAmbientColor()
{
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x / 4, m_resolution.y / 4);

	auto emptyVao = m_objectManager.getVertexArray("emptyVao");
	auto pPipeline = m_shaderManager.getPipeline("blurpassNew.pipe");

	/*vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindFrameBuffer(m_blurFB[0]);
	vx::gl::StateManager::bindVertexArray(emptyVao->getId());

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	m_pColdData->m_ambientColorTexture.bind();
	glActiveTexture(GL_TEXTURE0);

	glDrawArrays(GL_POINTS, 0, 1);*/

	pPipeline = m_shaderManager.getPipeline("blurpass.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindFrameBuffer(m_blurFB[0]);
	glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	//m_pColdData->m_ambientColorBlurTexture[0].bind();
	m_pColdData->m_ambientColorTexture.bind();
	glActiveTexture(GL_TEXTURE0);
	glDrawArrays(GL_POINTS, 0, 1);

	/*glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
	vx::gl::StateManager::bindFrameBuffer(m_blurFB[1]);
	m_pColdData->m_ambientColorBlurTexture[0].bind();
	glDrawArrays(GL_POINTS, 0, 1);*/

	pPipeline = m_shaderManager.getPipeline("blurpass2.pipe");
	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindFrameBuffer(m_coneTraceFB);

	glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
	m_pColdData->m_ambientColorBlurTexture[0].bind();
	glActiveTexture(GL_TEXTURE0);

	glDrawArrays(GL_POINTS, 0, 1);
}

void RenderAspect::renderProfiler()
{
	vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);
	m_textCmdList.draw();
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
	default:
		break;
	}
}

void RenderAspect::getProjectionMatrix(vx::mat4* m)
{
	*m = m_projectionMatrix;
}

u16 RenderAspect::addActorToBuffer(const vx::StringID &actorSid, const vx::Transform &transform, const vx::StringID &mesh, const vx::StringID &material)
{
	vx::gl::DrawElementsIndirectCommand drawCmd;
	u32 cmdIndex = 0;

	u32 materialIndex = 0;
	m_materialManager.getMaterialIndex(material, m_fileAspect,&materialIndex);
	auto gpuIndex = m_meshManager.addMeshInstance(actorSid, transform, mesh, materialIndex, m_fileAspect);
	//auto gpuIndex = m_sceneRenderer.addActorToBuffer(transform, mesh, material, &drawCmd, &cmdIndex, m_fileAspect);

	/*auto count = m_sceneRenderer.getMeshInstanceCount();
	auto buffer = m_objectManager.getBuffer("meshParamBuffer");
	buffer->subData(0, sizeof(u32), &count);*/

	return gpuIndex;
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