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
#include <vxRenderAspect/EditorRenderAspect.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxgl/gl.h>
#include <vxLib/File/FileHandle.h>
#include <vxRenderAspect/Graphics/Renderer.h>
#include <vxRenderAspect/Graphics/CommandListFactory.h>
#include <vxRenderAspect/DDS_File.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/NavMeshGraph.h>
#include <UniformCameraBuffer.h>
#include <vxGL/StateManager.h>
#include <EditorLightBuffer.h>
#include <vxRenderAspect/gl/BufferBindingManager.h>
#include <UniformCameraBufferStatic.h>
#include <vxEngineLib/Light.h>
#include <vxGL/Debug.h>

struct InfluenceCellVertex
{
	vx::float3 position;
	u32 count;
};

namespace Editor
{
	//enum class RenderAspect::EditorUpdate : u32{ Update_None, Update_Mesh, Update_Material, Editor_Added_Instance, Editor_Update_Instance, Editor_Set_Scene };

		void __stdcall renderDebugCallback()
		{
			std::abort();
		}

	const u32 g_maxLightCount{128};

	struct VertexPositionColor
	{
		vx::float3 position;
		vx::float3 color;
	};

	struct RenderAspect::ColdData
	{
		u32 m_lightCount;
		vx::gl::Texture m_texture;
	};

	RenderAspect::RenderAspect()
		:m_fileAspect(nullptr)
	{

	}

	RenderAspect::~RenderAspect()
	{

	}

	RenderAspectInitializeError RenderAspect::initialize(const RenderAspectDescription &renderDesc)
	{
		auto resolution = renderDesc.settings->m_resolution;
		f32 znear = renderDesc.settings->m_zNear;
		f32 zfar = renderDesc.settings->m_zFar;
		m_resolution = resolution;
		m_projectionMatrix = vx::MatrixPerspectiveFovRHDX(vx::degToRad(renderDesc.settings->m_fov), (f32)resolution.x / (f32)resolution.y, znear, zfar);

		m_fileAspect = renderDesc.fileAspect;

		vx::gl::ContextDescription contextDesc;
		contextDesc.tmpHwnd = (HWND)renderDesc.tmpHwnd;
		contextDesc.glParams.hwnd = (HWND)renderDesc.hwnd;
		contextDesc.glParams.resolution = resolution;
		contextDesc.glParams.majVersion = 4;
		contextDesc.glParams.minVersion = 5;
		contextDesc.glParams.bVsync = 0;
		contextDesc.glParams.bDebugMode = 1;

		if (!m_renderContext.initialize(contextDesc))
		{
			return RenderAspectInitializeError::ERROR_CONTEXT;
		}

		vx::gl::Debug::initialize();
		vx::gl::Debug::setHighSeverityCallback(renderDebugCallback);
		vx::gl::Debug::enableCallback(true);

		const auto memorySize = 5 MBYTE;
		auto memory = renderDesc.pAllocator->allocate(memorySize, 16);
		if (memory == nullptr)
		{
			return RenderAspectInitializeError::ERROR_OUT_OF_MEMORY;
		}

		m_allocator = vx::StackAllocator(memory, memorySize);
		m_coldData = vx::make_unique<ColdData>();

		vx::gl::StateManager::disable(vx::gl::Capabilities::Framebuffer_sRGB);
		vx::gl::StateManager::enable(vx::gl::Capabilities::Texture_Cube_Map_Seamless);
		vx::gl::StateManager::setClearColor(0, 0, 0, 1);
		vx::gl::StateManager::setViewport(0, 0, m_resolution.x, m_resolution.y);

		m_objectManager.initialize(50, 20, 20, 20, &m_allocator);
		m_camera.setPosition(0, 5, 5);

		if (!m_shaderManager.initialize(renderDesc.dataDir, &m_allocator, false))
		{
			return RenderAspectInitializeError::ERROR_SHADER;
		}

		m_objectManager.createVertexArray("emptyVao");

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
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Shader_Storage_Buffer;
			desc.immutable = 1;
			desc.size = sizeof(LightData) * g_maxLightCount + sizeof(vx::uint4);
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.pData = nullptr;

			m_objectManager.createBuffer("editorLightBuffer", desc);
		}

		m_shaderManager.setDefine("NOSHADOWS");
		m_shaderManager.addParameter("maxShadowLights", 5);

		auto shaderIncludeDir = renderDesc.dataDir + "shaders/include/";
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
		m_shaderManager.addIncludeFile((shaderIncludeDir + "EditorLightBuffer.h").c_str(), "EditorLightBuffer.h");

		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawSpawn.pipe"), "editorDrawSpawn.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawPointColor.pipe"), "editorDrawPointColor.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawPoint.pipe"), "editorDrawPoint.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawInfluenceCell.pipe"), "editorDrawInfluenceCell.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawLights.pipe"), "editorDrawLights.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawMesh.pipe"), "editorDrawMesh.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("navmesh.pipe"), "navmesh.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorDrawNavmeshConnection.pipe"), "editorDrawNavmeshConnection.pipe", &m_allocator);
		m_shaderManager.loadPipeline(vx::FileHandle("editorSelectedMesh.pipe"), "editorSelectedMesh.pipe", &m_allocator);

		Graphics::Renderer::provide(&m_shaderManager, &m_objectManager, renderDesc.settings, nullptr);

		m_sceneRenderer.initialize(10, &m_objectManager, renderDesc.pAllocator);

		{
			vx::gl::BufferDescription navmeshVertexVboDesc;
			navmeshVertexVboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
			navmeshVertexVboDesc.flags = vx::gl::BufferStorageFlags::Write;
			navmeshVertexVboDesc.immutable = 1;
			navmeshVertexVboDesc.pData = nullptr;
			navmeshVertexVboDesc.size = sizeof(VertexPositionColor) * 256;
			m_objectManager.createBuffer("navMeshVertexVbo", navmeshVertexVboDesc);
		}

		{
			vx::gl::BufferDescription navMeshVertexIboDesc;
			navMeshVertexIboDesc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
			navMeshVertexIboDesc.flags = vx::gl::BufferStorageFlags::Write;
			navMeshVertexIboDesc.immutable = 1;
			navMeshVertexIboDesc.pData = nullptr;
			navMeshVertexIboDesc.size = sizeof(u16) * 256 * 3;
			m_objectManager.createBuffer("navMeshVertexIbo", navMeshVertexIboDesc);
		}

		createNavMeshVertexVao();
		createNavMeshVao();

		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(vx::float3) * 256;
			m_objectManager.createBuffer("navMeshGraphNodesVbo", desc);
		}

		createNavMeshNodesVao();


		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(vx::float3) * 256;
			m_objectManager.createBuffer("spawnPointVbo", desc);
		}

		auto spawnPointVbo = m_objectManager.getBuffer("spawnPointVbo");

		m_objectManager.createVertexArray("spawnPointVao");
		auto m_spawnPointVao = m_objectManager.getVertexArray("spawnPointVao");

		m_spawnPointVao->create();
		m_spawnPointVao->enableArrayAttrib(0);
		m_spawnPointVao->arrayAttribBinding(0, 0);
		m_spawnPointVao->arrayAttribFormatF(0, 3, 0, 0);
		m_spawnPointVao->bindVertexBuffer(*spawnPointVbo, 0, 0, sizeof(vx::float3));

		createIndirectCmdBuffers();

		{
			auto vaoSid = m_objectManager.createVertexArray("drawInfluenceCellNewVao");

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(VertexPositionColor) * 256 * 3;

			auto vboSid = m_objectManager.createBuffer("drawInfluenceCellNewVbo", desc);

			vx::gl::DrawArraysIndirectCommand cmd;
			cmd.baseInstance = 0;
			cmd.count = 0;
			cmd.first = 0;
			cmd.instanceCount = 1;

			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.pData = &cmd;
			desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);
			m_objectManager.createBuffer("drawInfluenceCellNewCmd", desc);

			auto vao = m_objectManager.getVertexArray(vaoSid);
			auto vbo = m_objectManager.getBuffer(vboSid);
			auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

			vao->enableArrayAttrib(0);
			vao->enableArrayAttrib(1);

			vao->arrayAttribFormatF(0, 3, 0, 0);
			vao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));

			vao->arrayAttribBinding(0, 0);
			vao->arrayAttribBinding(1, 0);

			vao->bindVertexBuffer(*vbo, 0, 0, sizeof(VertexPositionColor));
			vao->bindIndexBuffer(*ibo);
		}

		{
			vx::gl::DrawArraysIndirectCommand cmd = {};
			cmd.instanceCount = 1;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
			desc.immutable = 1;
			desc.pData = &cmd;
			desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);

			m_objectManager.createBuffer("waypointCmdBuffer", desc);
		}

		{
			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Array_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = nullptr;
			desc.size = sizeof(VertexPositionColor) * 256;

			m_objectManager.createBuffer("waypointVbo", desc);
		}

		{
			auto vaoSid = m_objectManager.createVertexArray("waypointVao");

			auto vao = m_objectManager.getVertexArray(vaoSid);
			auto vbo = m_objectManager.getBuffer("waypointVbo");

			vao->enableArrayAttrib(0);
			vao->enableArrayAttrib(1);

			vao->arrayAttribFormatF(0, 3, 0, 0);
			vao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));

			vao->arrayAttribBinding(0, 0);
			vao->arrayAttribBinding(1, 0);

			vao->bindVertexBuffer(*vbo, 0, 0, sizeof(VertexPositionColor));
		}

		m_commandList.initialize();
		createCommandList();

		createEditorTextures();

		bindBuffers();

		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);

		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		updateCamera();

		return RenderAspectInitializeError::OK;
	}

	void RenderAspect::bindBuffers()
	{
		auto editorTextureBuffer = m_objectManager.getBuffer("editorTextureBuffer");
		auto editorLightBuffer = m_objectManager.getBuffer("editorLightBuffer");

		auto pCameraBufferStatic = m_objectManager.getBuffer("CameraBufferStatic");
		auto transformBuffer = m_objectManager.getBuffer("transformBuffer");
		auto materialBlockBuffer = m_objectManager.getBuffer("materialBlockBuffer");
		auto pTextureBuffer = m_objectManager.getBuffer("TextureBuffer");

		gl::BufferBindingManager::bindBaseUniform(0, m_cameraBuffer.getId());
		gl::BufferBindingManager::bindBaseUniform(2, pCameraBufferStatic->getId());

		glBindBufferBase(GL_UNIFORM_BUFFER, 8, editorTextureBuffer->getId());

		gl::BufferBindingManager::bindBaseShaderStorage(0, transformBuffer->getId());
		gl::BufferBindingManager::bindBaseShaderStorage(1, materialBlockBuffer->getId());
		gl::BufferBindingManager::bindBaseShaderStorage(2, pTextureBuffer->getId());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, editorLightBuffer->getId());
	}

	void RenderAspect::createEditorTextures()
	{
		vx::gl::TextureDescription desc;
		desc.format = vx::gl::TextureFormat::SRGBA_DXT5;
		desc.miplevels = 1;
		desc.size = vx::ushort3(512, 512, 2);
		desc.sparse = 0;
		desc.type = vx::gl::TextureType::Texture_2D_Array;

		m_coldData->m_texture.create(desc);

		DDS_File ddsFileLight;
		DDS_File ddsFileSpawn;
		if (!ddsFileLight.loadFromFile("../../data/textures/editor/light.dds") ||
			!ddsFileSpawn.loadFromFile("../../data/textures/editor/spawnPoint.dds"))
		{
			puts("Error loading texture !");
			VX_ASSERT(false);
			return;
		}

		auto &textureLight = ddsFileLight.getTexture(0);
		auto &textureSpawn = ddsFileSpawn.getTexture(0);

		vx::gl::TextureCompressedSubImageDescription subImgDesc;
		subImgDesc.dataSize = textureLight.getSize();
		subImgDesc.miplevel = 0;
		subImgDesc.offset = vx::ushort3(0);
		subImgDesc.p = textureLight.getPixels();
		subImgDesc.size = vx::ushort3(512, 512, 1);
		m_coldData->m_texture.subImageCompressed(subImgDesc);

		subImgDesc.offset = vx::ushort3(0, 0, 1);
		subImgDesc.dataSize = textureSpawn.getSize();
		subImgDesc.p = textureSpawn.getPixels();
		m_coldData->m_texture.subImageCompressed(subImgDesc);

		auto handle = m_coldData->m_texture.getTextureHandle();
		m_coldData->m_texture.makeTextureResident();

		vx::gl::BufferDescription bufferDesc;
		bufferDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
		bufferDesc.flags = 0;
		bufferDesc.immutable = 1;
		bufferDesc.pData = &handle;
		bufferDesc.size = sizeof(u64);

		m_objectManager.createBuffer("editorTextureBuffer", bufferDesc);
	}

	void RenderAspect::createNavMeshNodesVao()
	{
		m_objectManager.createVertexArray("navMeshGraphNodesVao");
		auto navMeshGraphNodesVao = m_objectManager.getVertexArray("navMeshGraphNodesVao");
		navMeshGraphNodesVao->create();

		navMeshGraphNodesVao->enableArrayAttrib(0);
		navMeshGraphNodesVao->arrayAttribFormatF(0, 3, 0, 0);
		navMeshGraphNodesVao->arrayAttribBinding(0, 0);

		auto navMeshGraphNodesVbo = m_objectManager.getBuffer("navMeshGraphNodesVbo");
		navMeshGraphNodesVao->bindVertexBuffer(*navMeshGraphNodesVbo, 0, 0, sizeof(vx::float3));
	}

	void RenderAspect::createIndirectCmdBuffers()
	{
		{
			vx::gl::DrawArraysIndirectCommand arrayCmd = {};
			arrayCmd.instanceCount = 1;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = &arrayCmd;
			desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);

			m_objectManager.createBuffer("editorLightCmdBuffer", desc);
			m_objectManager.createBuffer("navMeshVertexCmdBuffer", desc);
			m_objectManager.createBuffer("graphNodesCmdBuffer", desc);
			m_objectManager.createBuffer("spawnPointCmdBuffer", desc);
		}

		{
			vx::gl::DrawElementsIndirectCommand elementsCmd;
			memset(&elementsCmd, 0, sizeof(vx::gl::DrawElementsIndirectCommand));
			elementsCmd.instanceCount = 1;

			vx::gl::BufferDescription desc;
			desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
			desc.flags = vx::gl::BufferStorageFlags::Write;
			desc.immutable = 1;
			desc.pData = &elementsCmd;
			desc.size = sizeof(vx::gl::DrawElementsIndirectCommand);

			m_objectManager.createBuffer("navmeshCmdBuffer", desc);
		}
	}

	void RenderAspect::createNavMeshVao()
	{
		auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");
		auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

		m_objectManager.createVertexArray("navMeshVao");
		auto navMeshVao = m_objectManager.getVertexArray("navMeshVao");
		navMeshVao->enableArrayAttrib(0);
		navMeshVao->arrayAttribFormatF(0, 3, 0, 0);
		navMeshVao->arrayAttribBinding(0, 0);
		navMeshVao->bindVertexBuffer(*navMeshVertexVbo, 0, 0, sizeof(VertexPositionColor));

		navMeshVao->bindIndexBuffer(*ibo);
	}

	void RenderAspect::createNavMeshVertexVao()
	{
		auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");

		m_objectManager.createVertexArray("navMeshVertexVao");
		auto navMeshVertexVao = m_objectManager.getVertexArray("navMeshVertexVao");

		navMeshVertexVao->create();
		navMeshVertexVao->enableArrayAttrib(0);
		navMeshVertexVao->arrayAttribFormatF(0, 3, 0, 0);
		navMeshVertexVao->arrayAttribBinding(0, 0);

		navMeshVertexVao->enableArrayAttrib(1);
		navMeshVertexVao->arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
		navMeshVertexVao->arrayAttribBinding(1, 0);

		navMeshVertexVao->bindVertexBuffer(*navMeshVertexVbo, 0, 0, sizeof(VertexPositionColor));
	}

	void RenderAspect::createCommandList()
	{
		vx::gl::BufferDescription bufferDesc;
		bufferDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		bufferDesc.flags = vx::gl::BufferStorageFlags::Write;
		bufferDesc.immutable = 1;
		bufferDesc.size = sizeof(vx::float3) * 2 * 1024;

		auto bufferSid = m_objectManager.createBuffer("editorDrawNavmeshConnectionVbo", bufferDesc);

		vx::gl::DrawArraysIndirectCommand cmd;
		cmd.baseInstance = 0;
		cmd.count = 0;
		cmd.first = 0;
		cmd.instanceCount = 1;

		bufferDesc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		bufferDesc.size = sizeof(vx::gl::DrawArraysIndirectCommand);
		bufferDesc.pData = &cmd;
		bufferDesc.flags = vx::gl::BufferStorageFlags::Write;
		auto cmdSid = m_objectManager.createBuffer("editorDrawNavmeshConnectionCmd", bufferDesc);
		auto vaoSid = m_objectManager.createVertexArray("editorDrawNavmeshConnectionVao");

		auto vao = m_objectManager.getVertexArray(vaoSid);
		auto vbo = m_objectManager.getBuffer(bufferSid);

		vao->enableArrayAttrib(0);
		vao->arrayAttribFormatF(0, 3, 0, 0);
		vao->arrayAttribBinding(0, 0);
		vao->bindVertexBuffer(*vbo, 0, 0, sizeof(vx::float3));

		Graphics::CommandListFactory::createFromFile("commandListEditor.txt", m_objectManager, m_shaderManager, &m_commandList);
	}

	void RenderAspect::shutdown(void* hwnd)
	{
		m_sceneRenderer.shutdown();
		m_objectManager.shutdown();
		m_coldData.reset(nullptr);
		m_renderContext.shutdown((HWND)hwnd);
	}

	bool RenderAspect::initializeProfiler()
	{
		return true;
	}

	void RenderAspect::makeCurrent(bool b)
	{

	}

	void RenderAspect::queueUpdateTask(const RenderUpdateTask &task)
	{

	}

	void RenderAspect::queueUpdateTask(const RenderUpdateTask &task, const u8* data, u32 dataSize)
	{

	}

	void RenderAspect::queueUpdateCamera(const RenderUpdateCameraData &data)
	{

	}

	void RenderAspect::updateCamera()
	{
		auto projectionMatrix = m_projectionMatrix;

		UniformCameraBufferBlock block;
		m_camera.getViewMatrix(&block.viewMatrix);
		block.pvMatrix = projectionMatrix * block.viewMatrix;
		block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
		block.position = m_camera.getPosition();

		m_cameraBuffer.subData(0, sizeof(UniformCameraBufferBlock), &block);
	}

	void RenderAspect::update()
	{
		updateCamera();
	}

	void RenderAspect::updateProfiler(f32 dt)
	{

	}

	void RenderAspect::submitCommands()
	{
		vx::gl::StateManager::bindFrameBuffer(0);
		vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (m_selectedInstance.ptr != nullptr)
		{
			auto cmdBuffer = m_objectManager.getBuffer("meshCmdBuffer");
			auto meshVao = m_objectManager.getVertexArray("meshVao");

			vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

			vx::gl::StateManager::bindVertexArray(*meshVao);

			vx::gl::StateManager::bindBuffer(vx::gl::BufferType::Draw_Indirect_Buffer, cmdBuffer->getId());

			auto offset = m_selectedInstance.cmd.baseInstance * sizeof(vx::gl::DrawElementsIndirectCommand);

			auto pipe = m_shaderManager.getPipeline("editorSelectedMesh.pipe");
			vx::gl::StateManager::bindPipeline(*pipe);

			glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)offset);
		}

		m_commandList.draw();
	}

	void RenderAspect::endFrame()
	{
		glFinish();
		m_renderContext.swapBuffers();
	}

	void RenderAspect::handleEvent(const vx::Event &evt)
	{
		switch (evt.type)
		{
		case(vx::EventType::File_Event) :
			handleFileEvent(evt);
			break;
		case(vx::EventType::Editor_Event) :
			//handleEditorEvent(evt);
			break;
		default:
			break;
		}
	}

	void RenderAspect::keyPressed(u16 key)
	{

	}

	void RenderAspect::getProjectionMatrix(vx::mat4* m)
	{
		*m = m_projectionMatrix;
	}

	void RenderAspect::getTotalVRam(u32* totalVram) const
	{

	}

	void RenderAspect::getTotalAvailableVRam(u32* totalAvailableVram) const
	{

	}

	void RenderAspect::getAvailableVRam(u32* availableVram) const
	{
		*availableVram = 0;
	}

	void RenderAspect::addMeshInstance(const Editor::MeshInstance &instance)
	{
		m_sceneRenderer.editorAddMeshInstance(instance.getMeshInstance(), m_fileAspect);
	}

	bool RenderAspect::removeMeshInstance(const vx::StringID &sid)
	{
		return m_sceneRenderer.editorRemoveStaticMeshInstance(sid);
	}

	bool RenderAspect::setSelectedMeshInstance(const Editor::MeshInstance* instance)
	{
		if (instance)
		{
			auto cmd = m_sceneRenderer.getDrawCommand(instance->getNameSid());

			if (cmd.count == 0)
			{
				return false;
			}

			m_selectedInstance.cmd = cmd;
			m_selectedInstance.ptr = instance;
		}
		else
		{
			m_selectedInstance.ptr = nullptr;
			return false;
		}

		return true;
	}

	void RenderAspect::setSelectedMeshInstanceTransform(vx::Transform &transform)
	{
		if (m_selectedInstance.ptr)
		{
			auto index = m_selectedInstance.cmd.baseInstance;

			m_sceneRenderer.updateTransform(transform, index);
		}
	}

	bool RenderAspect::setSelectedMeshInstanceMaterial(const Reference<Material> &material) const
	{
		return m_sceneRenderer.setMeshInstanceMaterial(m_selectedInstance.ptr->getNameSid(), material);
	}

	bool RenderAspect::setMeshInstanceMesh(const vx::StringID &sid, const vx::StringID &meshSid)
	{
		return m_sceneRenderer.setMeshInstanceMesh(sid, meshSid, m_fileAspect);
	}

	void RenderAspect::moveCamera(f32 dirX, f32 dirY, f32 dirZ)
	{
		const f32 speed = 0.05f;

		__m128 direction = { dirX, dirY, dirZ, 0 };
		m_camera.move(direction, speed);
	}

	void VX_CALLCONV RenderAspect::rotateCamera(const __m128 rotation)
	{
		m_camera.setRotation(rotation);
	}

	void RenderAspect::uploadToNavMeshVertexBuffer(const VertexPositionColor* vertices, u32 count)
	{
		auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");

		auto dst = navMeshVertexVbo->map<VertexPositionColor>(vx::gl::Map::Write_Only);
		::memcpy(dst.get(), vertices, sizeof(VertexPositionColor) * count);
	}

	void RenderAspect::updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, u32 count, u32(&selectedVertexIndex)[3], u8 selectedCount)
	{
		auto color = vx::float3(1, 0, 0);
		auto src = vx::make_unique<VertexPositionColor[]>(count);
		for (u32 i = 0; i < count; ++i)
		{
			src[i].position = vertices[i];
			src[i].color = color;
		}

		for (u8 i = 0; i < selectedCount; ++i)
		{
			auto index = selectedVertexIndex[i];
			src[index].color.y = 1.0f;
		}

		//m_coldData->m_navMeshVertexCount = count;
		uploadToNavMeshVertexBuffer(src.get(), count);

		auto cmdBuffer = m_objectManager.getBuffer("navMeshVertexCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
	}

	void RenderAspect::updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertexIndex)[3], u8 selectedCount)
	{
		auto navMeshVertexCount = navMesh.getVertexCount();
		if (navMeshVertexCount != 0)
		{
			auto vertices = navMesh.getVertices();
			updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, selectedVertexIndex, selectedCount);
		}

		updateNavMeshIndexBuffer(navMesh);
	}

	void RenderAspect::updateNavMeshBuffer(const NavMesh &navMesh)
	{
		auto navMeshVertexCount = navMesh.getVertexCount();
		if (navMeshVertexCount != 0)
		{
			auto vertices = navMesh.getVertices();
			u32 tmp[3];
			updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, tmp, 0);
		}

		updateNavMeshIndexBuffer(navMesh);
	}

	void RenderAspect::updateInfluenceCellBuffer(const InfluenceMap &influenceMap)
	{
		auto influenceCells = influenceMap.getCells();
		auto cellCount = influenceMap.getCellCount();
		if (cellCount == 0)
			return;

		auto triangles = influenceMap.getTriangles();
		printf("InfluenceCellsNew: %u\n", cellCount);

		auto vbo = m_objectManager.getBuffer("drawInfluenceCellNewVbo");
		auto mappedBuffer = vbo->map<VertexPositionColor>(vx::gl::Map::Write_Only);

		const vx::float3 colors[] =
		{
			{ 1, 0, 0 },
			{ 0.5f, 1, 0 },
			{ 0.5f, 0, 1 },
			{ 0, 1, 0 },
			{ 1, 0.5f, 0 },
			{ 0, 0.5f, 1 },
			{ 0, 0, 1 },
			{ 0, 1, 0.5f },
			{ 1, 0, 0.5f },
			{ 1, 0.5f, 1 },
			{ 1, 1, 0 },
			{ 0, 1, 1 },
			{ 1, 0, 1 }
		};

		const u32 colorCount = sizeof(colors) / sizeof(vx::float3);

		u32 vertexOffset = 0;
		u32 colorIndex = 0;
		for (u32 i = 0; i < cellCount; ++i)
		{
			auto &cell = influenceCells[i];

			for (u32 k = 0; k < cell.triangleCount; ++k)
			{
				auto triangleIndex = cell.triangleOffset + k;
				auto &triangle = triangles[triangleIndex];

				mappedBuffer[vertexOffset + 0].position = triangle[0];
				mappedBuffer[vertexOffset + 0].color = colors[colorIndex];

				mappedBuffer[vertexOffset + 1].position = triangle[1];
				mappedBuffer[vertexOffset + 1].color = colors[colorIndex];

				mappedBuffer[vertexOffset + 2].position = triangle[2];
				mappedBuffer[vertexOffset + 2].color = colors[colorIndex];

				vertexOffset += 3;
			}

			colorIndex = (colorIndex + 1) % colorCount;
		}

		auto cmdBuffer = m_objectManager.getBuffer("drawInfluenceCellNewCmd");
		auto mappedCmd = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmd->count = vertexOffset;
	}

	void RenderAspect::updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph)
	{
		auto nodes = navMeshGraph.getNodes();
		auto nodeCount = navMeshGraph.getNodeCount();

		if (nodeCount == 0)
			return;

		auto src = vx::make_unique < vx::float3[]>(nodeCount);
		for (u32 i = 0; i < nodeCount; ++i)
		{
			src[i] = nodes[i].m_position;
		}

		//	m_navMeshGraphNodesCount = nodeCount;

		auto navMeshGraphNodesVbo = m_objectManager.getBuffer("navMeshGraphNodesVbo");
		auto mappedBuffer = navMeshGraphNodesVbo->map<vx::float3>(vx::gl::Map::Write_Only);
		::memcpy(mappedBuffer.get(), src.get(), sizeof(vx::float3) * nodeCount);

		auto cmdBuffer = m_objectManager.getBuffer("graphNodesCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = nodeCount;

		{
			auto connections = navMeshGraph.getConnections();
			auto connectionCount = navMeshGraph.getConnectionCount();
			VX_ASSERT(connectionCount < 1024);

			u32 index = 0;
			auto vbo = m_objectManager.getBuffer("editorDrawNavmeshConnectionVbo");
			auto cmd = m_objectManager.getBuffer("editorDrawNavmeshConnectionCmd");

			auto mappedVbo = vbo->map<vx::float3>(vx::gl::Map::Write_Only);

			for (u32 i = 0; i < nodeCount; ++i)
			{
				auto &currentNode = nodes[i];
				for (u32 j = 0; j < currentNode.m_connectionCount; ++j)
				{
					auto &connection = connections[currentNode.m_connectionOffset + j];
					//VX_ASSERT(otherIndex < nodeCount);
					auto &otherNode = nodes[connection.m_toNode];

					mappedVbo[index++] = currentNode.m_position;
					mappedVbo[index++] = otherNode.m_position;
				}
			}

			auto mappedCmd = cmd->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmd->count = index;
		}
	}

	void RenderAspect::updateLightBuffer(const Light* lights, u32 count)
	{
		count = std::min(count, g_maxLightCount);

		auto editorLightBuffer = m_objectManager.getBuffer("editorLightBuffer");
		auto mappedLightBuffer = editorLightBuffer->mapRange<LightData>(sizeof(vx::uint4), sizeof(LightData) * count, vx::gl::MapRange::Write);
		for (u32 i = 0;i < count; ++i)
		{
			mappedLightBuffer[i].position.x = lights[i].m_position.x;
			mappedLightBuffer[i].position.y = lights[i].m_position.y;
			mappedLightBuffer[i].position.z = lights[i].m_position.z;
			mappedLightBuffer[i].falloff= lights[i].m_falloff;
			mappedLightBuffer[i].lumen = lights[i].m_lumen;
		}
		mappedLightBuffer.unmap();

		auto mappedLightBufferCount = editorLightBuffer->mapRange<vx::uint4>(0, sizeof(vx::uint4), vx::gl::MapRange::Write);
		mappedLightBufferCount->x = count;
		mappedLightBufferCount.unmap();

		auto cmdBuffer = m_objectManager.getBuffer("editorLightCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
	}

	void RenderAspect::updateWaypoints(const Waypoint* w, u32 count)
	{
		auto vbo = m_objectManager.getBuffer("waypointVbo");
		auto cmd = m_objectManager.getBuffer("waypointCmdBuffer");

		auto mappedVbo = vbo->map<VertexPositionColor>(vx::gl::Map::Write_Only);
		for (u32 i = 0; i < count; ++i)
		{
			mappedVbo[i].position = w[i].position;
			mappedVbo[i].color = { 1, 1, 0 };
		}
		mappedVbo.unmap();

		cmd->subData(0, sizeof(u32), &count);
	}

	void RenderAspect::updateSpawns(const Spawn* spawns, u32 count)
	{
		auto cmdBuffer = m_objectManager.getBuffer("spawnPointCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
		mappedCmdBuffer.unmap();

		auto spawnPointVbo = m_objectManager.getBuffer("spawnPointVbo");
		auto mappedVbo = spawnPointVbo->map<vx::float3>(vx::gl::Map::Write_Only);
		for (u32 i = 0; i < count; ++i)
		{
			mappedVbo[i] = spawns[i].position;
		}
	}

	const vx::Camera& RenderAspect::getCamera() const
	{
		return m_camera;
	}

	void RenderAspect::updateNavMeshIndexBuffer(const u16* indices, u32 triangleCount)
	{
		auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

		auto dst = ibo->map<u16>(vx::gl::Map::Write_Only);
		::memcpy(dst.get(), indices, sizeof(u16) * triangleCount * 3);
	}

	void RenderAspect::updateNavMeshIndexBuffer(const NavMesh &navMesh)
	{
		auto triangleCount = navMesh.getTriangleCount();
		if (triangleCount != 0)
		{
			auto indices = navMesh.getTriangleIndices();
			updateNavMeshIndexBuffer(indices, triangleCount);

			auto navMeshIndexCount = triangleCount * 3;
			//m_pEditorColdData->m_navMeshIndexCount = navMeshIndexCount;

			auto navmeshCmdBuffer = m_objectManager.getBuffer("navmeshCmdBuffer");
			auto mappedCmdBuffer = navmeshCmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmdBuffer->count = navMeshIndexCount;
		}
	}

	void RenderAspect::handleLoadScene(const vx::Event &evt)
	{
		auto scene = (Editor::Scene*)evt.arg2.ptr;

		m_sceneRenderer.loadScene(scene, m_objectManager, m_fileAspect);

		auto count = m_sceneRenderer.getMeshInstanceCount();
		auto buffer = m_objectManager.getBuffer("meshParamBuffer");
		buffer->subData(0, sizeof(u32), &count);

		auto lightCount = scene->getLightCount();
		m_coldData->m_lightCount = lightCount;
		auto &navMesh = scene->getNavMesh();

		updateNavMeshBuffer(navMesh);

		{
			auto cmdBuffer = m_objectManager.getBuffer("editorLightCmdBuffer");
			auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
			mappedCmdBuffer->count = lightCount;
			mappedCmdBuffer.unmap();
		}

		auto spawnCount = scene->getSpawnCount();
		auto spawns = scene->getSpawns();
		updateSpawns(spawns, spawnCount);
	}

	void RenderAspect::handleFileEvent(const vx::Event &evt)
	{
		if ((vx::FileEvent)evt.code == vx::FileEvent::EditorScene_Loaded)
		{
			handleLoadScene(evt);
		}
		else if ((vx::FileEvent)evt.code == vx::FileEvent::Mesh_Loaded)
		{
			//handleLoadMesh(evt);
		}
	}
}