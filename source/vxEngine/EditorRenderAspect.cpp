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
#include "EditorRenderAspect.h"
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include "Scene.h"
#include <vxLib/gl/gl.h>
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/util/DebugPrint.h>
#include "developer.h"
#include "GpuStructs.h"
#include "InfluenceMap.h"
#include "NavMeshGraph.h"
#include <vxLib/algorithm.h>
#include "Graphics/Segment.h"
#include "DDS_File.h"
#include "Spawn.h"
#include "Graphics/Commands.h"
#include "Graphics/Segment.h"
#include "Graphics/Commands/ProgramUniformCommand.h"
#include "NavMeshTriangle.h"
#include "SegmentFactory.h"
#include "Graphics/CommandListFactory.h"
#include "EngineConfig.h"
#include "EditorMeshInstance.h"

struct VertexNavMesh
{
	vx::float3 position;
	vx::float3 color;
};

struct InfluenceCellVertex
{
	vx::float3 position;
	u32 count;
};

enum class EditorRenderAspect::EditorUpdate : u32{ Update_None, Update_Mesh, Update_Material, Editor_Added_Instance, Editor_Update_Instance, Editor_Set_Scene };

EditorRenderAspect::EditorRenderAspect()
	:RenderAspect()
{
}

bool EditorRenderAspect::initialize(const std::string &dataDir, HWND panel, HWND tmp, vx::StackAllocator *pAllocator, const EngineConfig* settings)
{
	/*RenderAspect::createColdData();
	RenderAspect::provideRenderData(settings);

	if (!m_renderContext.initializeExtensions(tmp))
	{
		puts("Error initializing Extensions");
		return false;
	}

	vx::gl::OpenGLDescription glDescription;
	glDescription.bDebugMode = debug;
	glDescription.bVsync = vsync;
	glDescription.farZ = zFar;
	glDescription.fovRad = vx::degToRad(fovDeg);
	glDescription.hwnd = panel;
	glDescription.majVersion = 4;
	glDescription.minVersion = 5;
	glDescription.nearZ = zNear;
	glDescription.resolution = windowResolution;

	//vx::gl::OpenGLDescription glDesc = vx::gl::OpenGLDescription::create(panel, windowResolution, , zNear, zFar, 4, 5, vsync, debug);
	if (!m_renderContext.initializeOpenGl(glDescription))
	{
		puts("Error initializing Context");
		return false;
	}*/


	vx::gl::OpenGLDescription glDescription;
	glDescription.bDebugMode = settings->m_renderDebug;
	glDescription.bVsync = settings->m_vsync;
	glDescription.farZ = settings->m_zFar;
	glDescription.fovRad = vx::degToRad(settings->m_fov);
	glDescription.hwnd = panel;
	glDescription.majVersion = 4;
	glDescription.minVersion = 5;
	glDescription.nearZ = settings->m_zNear;
	glDescription.resolution = settings->m_resolution;
	vx::gl::ContextDescription contextDesc;
	contextDesc.tmpHwnd = tmp;
	contextDesc.glParams = glDescription;

	if (!initializeCommon(contextDesc, settings))
		return false;

	m_pEditorColdData = vx::make_unique<EditorColdData>();

	auto result = initializeImpl(dataDir, settings->m_resolution, settings->m_renderDebug, pAllocator);

	{
		vx::gl::BufferDescription navmeshVertexVboDesc;
		navmeshVertexVboDesc.bufferType = vx::gl::BufferType::Array_Buffer;
		navmeshVertexVboDesc.flags = vx::gl::BufferStorageFlags::Write;
		navmeshVertexVboDesc.immutable = 1;
		navmeshVertexVboDesc.pData = nullptr;
		navmeshVertexVboDesc.size = sizeof(VertexNavMesh) * 256;
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
		desc.size = sizeof(VertexNavMesh) * 256 * 3;

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

		vao->bindVertexBuffer(*vbo, 0, 0, sizeof(VertexNavMesh));
		vao->bindIndexBuffer(*ibo);
	}

	createCommandList();

	createEditorTextures();

	auto editorTextureBuffer = m_objectManager.getBuffer("editorTextureBuffer");

	bindBuffers();
	glBindBufferBase(GL_UNIFORM_BUFFER, 8, editorTextureBuffer->getId());

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return result;
}

void EditorRenderAspect::createNavMeshVao()
{
	auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");
	auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

	m_objectManager.createVertexArray("navMeshVao");
	auto navMeshVao = m_objectManager.getVertexArray("navMeshVao");
	navMeshVao->enableArrayAttrib(0);
	navMeshVao->arrayAttribFormatF(0, 3, 0, 0);
	navMeshVao->arrayAttribBinding(0, 0);
	navMeshVao->bindVertexBuffer(*navMeshVertexVbo, 0, 0, sizeof(VertexNavMesh));

	navMeshVao->bindIndexBuffer(*ibo);
}

void EditorRenderAspect::createNavMeshVertexVao()
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

	navMeshVertexVao->bindVertexBuffer(*navMeshVertexVbo, 0, 0, sizeof(VertexNavMesh));
}

void EditorRenderAspect::createNavMeshNodesVao()
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

void EditorRenderAspect::createIndirectCmdBuffers()
{
	vx::gl::DrawArraysIndirectCommand arrayCmd;
	memset(&arrayCmd, 0, sizeof(vx::gl::DrawArraysIndirectCommand));
	arrayCmd.instanceCount = 1;

	{
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write;
		desc.immutable = 1;
		desc.pData = &arrayCmd;
		desc.size = sizeof(vx::gl::DrawArraysIndirectCommand);

		m_objectManager.createBuffer("lightCmdBuffer", desc);
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

	{
		u32 drawCount = 0;
		vx::gl::BufferDescription desc;
		desc.bufferType = vx::gl::BufferType::Parameter_Buffer;
		desc.flags = vx::gl::BufferStorageFlags::Write | vx::gl::BufferStorageFlags::Dynamic_Storage;
		desc.immutable = 1;
		desc.pData = &drawCount;
		desc.size = sizeof(u32);

		m_objectManager.createBuffer("meshParamBuffer", desc);
	}
}

void EditorRenderAspect::createCommandList()
{
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
	}

	Graphics::CommandListFactory::createFromFile("commandListEditor.txt", m_objectManager, m_shaderManager, &m_commandList);
}

void EditorRenderAspect::createEditorTextures()
{
	vx::gl::TextureDescription desc;
	desc.format = vx::gl::TextureFormat::SRGBA_DXT5;
	desc.miplevels = 1;
	desc.size = vx::ushort3(512, 512, 2);
	desc.sparse = 0;
	desc.type = vx::gl::TextureType::Texture_2D_Array;

	m_pEditorColdData->m_editorTextures.create(desc);

	DDS_File ddsFileLight;
	DDS_File ddsFileSpawn;
	if (!ddsFileLight.loadFromFile("../../game/data/textures/editor/light.dds") ||
		!ddsFileSpawn.loadFromFile("../../game/data/textures/editor/spawnPoint.dds"))
	{
		puts("Error loading texture !");
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
	m_pEditorColdData->m_editorTextures.subImageCompressed(subImgDesc);

	subImgDesc.offset = vx::ushort3(0, 0, 1);
	subImgDesc.dataSize = textureSpawn.getSize();
	subImgDesc.p = textureSpawn.getPixels();
	m_pEditorColdData->m_editorTextures.subImageCompressed(subImgDesc);

	auto handle = m_pEditorColdData->m_editorTextures.getTextureHandle();
	m_pEditorColdData->m_editorTextures.makeTextureResident();

	vx::gl::BufferDescription bufferDesc;
	bufferDesc.bufferType = vx::gl::BufferType::Uniform_Buffer;
	bufferDesc.flags = 0;
	bufferDesc.immutable = 1;
	bufferDesc.pData = &handle;
	bufferDesc.size = sizeof(u64);

	m_objectManager.createBuffer("editorTextureBuffer", bufferDesc);
}

void EditorRenderAspect::update()
{
	if (m_updateEditor.load() != 0)
	{
		updateEditor();
		m_updateEditor.store(0);
	}

	updateCamera();
	RenderAspect::update();
}

void EditorRenderAspect::render()
{
	clearTextures();
	clearBuffers();

	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
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

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_renderContext.swapBuffers();
}

void EditorRenderAspect::handleEvent(const vx::Event &evt)
{
	switch (evt.type)
	{
	case(vx::EventType::File_Event) :
		handleFileEvent(evt);
		break;
	case(vx::EventType::Ingame_Event) :
		handleIngameEvent(evt);
		break;
	case(vx::EventType::Editor_Event) :
		handleEditorEvent(evt);
		break;
	default:
		break;
	}
}

void EditorRenderAspect::addMesh(const vx::StringID &sid)
{
	VX_UNREFERENCED_PARAMETER(sid);
	/*auto &sceneMeshes = m_pCurrentScene->getMeshes();
	auto it = sceneMeshes.find(sid);
	assert(it != sceneMeshes.end());

	const vx::Mesh* pMesh = *it;*/

	assert(false);

	//writeMeshToVertexBuffer(sid, pMesh, &m_editorVertexCount, &m_editorIndexCount, &m_editorMeshEntries);
	//writeMeshToVertexBuffer(sid, pMesh, &m_editorVertexCount, &m_editorIndexCount);
	//printf("vertex count: %u\n", m_editorVertexCount);
}

void EditorRenderAspect::addMaterial(const vx::StringID &sid)
{
	VX_UNREFERENCED_PARAMETER(sid);
	/*auto &sceneMaterials = m_pCurrentScene->getMaterials();

	auto it = sceneMaterials.find(sid);
	assert(it != sceneMaterials.end());

	Material* pMaterial = *it;
	VX_ASSERT(pMaterial != nullptr, "Material == nullptr");

	auto materialIndex = m_editorData.addMaterial(sid, pMaterial);

	createMaterial(pMaterial);
	writeMaterialToBuffer(pMaterial, materialIndex);*/
}

void EditorRenderAspect::addMeshInstanceToBuffers()
{
	VX_ASSERT(false);
	/*auto instanceCount = m_pCurrentScene->getMeshInstanceCount();
	if (instanceCount == 0)
	return;

	u32 batchIndexStart = 0;
	u32 batchInstanceCount = 0;
	u32 batchInstanceStart = 0;
	u32 totalInstanceCount = 0;

	m_editorMeshInstanceIndices.clear();

	std::vector<std::pair<vx::StringID, const MeshInstance*>> meshInstances;
	meshInstances.reserve(instanceCount);

	auto &sceneMeshInstances = m_pCurrentScene->getMeshInstancesSortedByName();
	for (auto i = 0u; i < instanceCount; ++i)
	{
	meshInstances.push_back(
	std::make_pair(sceneMeshInstances.keys()[i], sceneMeshInstances.data() + i)
	);
	}

	auto &sceneMaterials = m_pCurrentScene->getMaterials();
	std::sort(meshInstances.begin(), meshInstances.end(), [&](const std::pair<vx::StringID, const MeshInstance*> &lhs, const std::pair<vx::StringID, const MeshInstance*> &rhs)
	{
	Material &lhsMaterial = **sceneMaterials.find(lhs.second->getMaterialSid());
	Material &rhsMaterial = **sceneMaterials.find(rhs.second->getMaterialSid());

	return (lhsMaterial < rhsMaterial) ||
	(lhsMaterial == rhsMaterial && lhs.second->getMeshSid() < rhs.second->getMeshSid());
	});

	auto batchMeshSid = meshInstances[0].second->getMeshSid();

	u32 drawCount = 0;
	u32 batchIndexCount = m_editorMeshEntries.find(batchMeshSid)->indexCount;

	for (auto i = 0u; i < instanceCount; ++i)
	{
	auto currentMeshSid = meshInstances[i].second->getMeshSid();
	auto meshEntry = m_editorMeshEntries.find(currentMeshSid);
	auto pCurrentMaterial = m_fileAspect.getMaterial(meshInstances[i].second->getMaterialSid());
	auto materialIndex = *m_editorMaterialIndices.find(pCurrentMaterial);

	if (currentMeshSid != batchMeshSid)
	{
	vx::gl::DrawElementsIndirectCommand cmd;
	cmd.count = batchIndexCount;
	cmd.instanceCount = batchInstanceCount;
	cmd.firstIndex = batchIndexStart;
	cmd.baseVertex = 0;
	cmd.baseInstance = batchInstanceStart;

	auto pCmd = (vx::gl::DrawElementsIndirectCommand*)m_meshIndirectBuffer.map(vx::gl::Map::Write_Only);
	pCmd[drawCount] = cmd;
	m_meshIndirectBuffer.unmap();

	++drawCount;

	batchIndexStart += batchIndexCount;
	batchInstanceStart += batchInstanceCount;
	batchInstanceCount = 0;
	batchMeshSid = currentMeshSid;
	batchIndexCount = meshEntry->indexCount;
	}

	writeMeshInstanceTransform(meshInstances[i].second, i);
	writeMeshInstanceIdBuffer(i, materialIndex);
	writeMeshInstanceToCommandBuffer(*meshEntry, i);

	m_editorMeshInstanceIndices.insert(meshInstances[i].first, i);

	++batchInstanceCount;
	++totalInstanceCount;
	}

	vx::gl::DrawElementsIndirectCommand cmd;
	cmd.count = batchIndexCount;
	cmd.instanceCount = batchInstanceCount;
	cmd.firstIndex = batchIndexStart;
	cmd.baseVertex = 0;
	cmd.baseInstance = batchInstanceStart;

	vx::gl::DrawElementsIndirectCommand *pCmd = (vx::gl::DrawElementsIndirectCommand*)m_meshIndirectBuffer.map(vx::gl::Map::Write_Only);
	pCmd[drawCount] = cmd;
	m_meshIndirectBuffer.unmap();

	++drawCount;

	m_stageVoxelize.setDrawCount(drawCount);
	m_stageForwardRender.setDrawCount(totalInstanceCount);
	m_stageUpdateAABB.setDrawCount(drawCount);

	m_drawCount = drawCount;
	m_meshInstanceCount = totalInstanceCount;

	printf("drawcount: %u, totalInstanceCount: %u\n", m_drawCount, totalInstanceCount);*/
}

void EditorRenderAspect::updateInstance(const vx::StringID &sid)
{
	VX_UNREFERENCED_PARAMETER(sid);
}

void EditorRenderAspect::updateEditor()
{
	std::lock_guard<std::mutex> guard(m_updateDataMutex);
	/*for (auto &it : m_updateData)
	{
	switch (it.second)
	{
	case EditorUpdate::Update_Mesh:
	editor_addMesh(it.first.sid);
	break;
	case EditorUpdate::Update_Material:
	editor_addMaterial(it.first.sid);
	break;
	case EditorUpdate::Editor_Added_Instance:
	editor_addMeshInstanceToBuffers();
	break;
	case EditorUpdate::Editor_Update_Instance:
	editor_updateInstance(it.first.sid);
	break;
	default:
	break;
	}
	}*/
	m_updateData.clear();
}

void EditorRenderAspect::editor_moveCamera(f32 dirX, f32 dirY, f32 dirZ)
{
	const f32 speed = 0.05f;

	__m128 direction = { dirX, dirY, dirZ, 0 };
	m_camera.move(direction, speed);
}

void VX_CALLCONV EditorRenderAspect::editor_rotateCamera(const __m128 rotation)
{
	m_camera.setRotation(rotation);
}

void EditorRenderAspect::handleEditorEvent(const vx::Event &evt)
{
	VX_UNREFERENCED_PARAMETER(evt);
}

void EditorRenderAspect::handleLoadScene(const vx::Event &evt)
{
	auto scene = (Scene*)evt.arg1.ptr;
	auto lightCount = scene->getLightCount();
	m_pEditorColdData->m_lightCount = lightCount;
	auto &navMesh = scene->getNavMesh();

	updateNavMeshBuffer(navMesh);

	{
		auto paramBuffer = m_objectManager.getBuffer("meshParamBuffer");
		u32 count = scene->getMeshInstanceCount();
		paramBuffer->subData(0, sizeof(u32), &count);
	}

	{
		auto cmdBuffer = m_objectManager.getBuffer("lightCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = lightCount;
	}

	auto spawnCount = scene->getSpawnCount();
	auto spawns = scene->getSpawns();
	{
		auto cmdBuffer = m_objectManager.getBuffer("spawnPointCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = spawnCount;
		mappedCmdBuffer.unmap();

		auto spawnPointVbo = m_objectManager.getBuffer("spawnPointVbo");
		auto mappedVbo = spawnPointVbo->map<vx::float3>(vx::gl::Map::Write_Only);
		for (u32 i = 0; i < spawnCount; ++i)
		{
			mappedVbo[i] = spawns[i].position;
		}
	}
}

void EditorRenderAspect::handleFileEvent(const vx::Event &evt)
{
	RenderAspect::handleFileEvent(evt);

	if ((vx::FileEvent)evt.code == vx::FileEvent::Scene_Loaded)
	{
		handleLoadScene(evt);
	}
}

void EditorRenderAspect::updateNavMeshBuffer(const NavMesh &navMesh)
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

void EditorRenderAspect::updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertexIndex)[3], u8 selectedCount)
{
	auto navMeshVertexCount = navMesh.getVertexCount();
	if (navMeshVertexCount != 0)
	{
		auto vertices = navMesh.getVertices();
		updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, selectedVertexIndex, selectedCount);
	}

	updateNavMeshIndexBuffer(navMesh);
}

void EditorRenderAspect::uploadToNavMeshVertexBuffer(const VertexNavMesh* vertices, u32 count)
{
	auto navMeshVertexVbo = m_objectManager.getBuffer("navMeshVertexVbo");

	auto dst = navMeshVertexVbo->map<VertexNavMesh>(vx::gl::Map::Write_Only);
	vx::memcpy(dst.get(), vertices, count);
}

void EditorRenderAspect::updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, u32 count, u32(&selectedVertexIndex)[3], u8 selectedCount)
{
	auto color = vx::float3(1, 0, 0);
	auto src = vx::make_unique<VertexNavMesh[]>(count);
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

	m_pEditorColdData->m_navMeshVertexCount = count;
	uploadToNavMeshVertexBuffer(src.get(), count);

	auto cmdBuffer = m_objectManager.getBuffer("navMeshVertexCmdBuffer");
	auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
	mappedCmdBuffer->count = count;
}

void EditorRenderAspect::updateNavMeshIndexBuffer(const NavMesh &navMesh)
{
	auto triangleCount = navMesh.getTriangleCount();
	if (triangleCount != 0)
	{
		auto indices = navMesh.getTriangleIndices();
		updateNavMeshIndexBuffer(indices, triangleCount);

		auto navMeshIndexCount = triangleCount * 3;
		m_pEditorColdData->m_navMeshIndexCount = navMeshIndexCount;

		auto navmeshCmdBuffer = m_objectManager.getBuffer("navmeshCmdBuffer");
		auto mappedCmdBuffer = navmeshCmdBuffer->map<vx::gl::DrawElementsIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = navMeshIndexCount;
	}
}

void EditorRenderAspect::updateNavMeshIndexBuffer(const u16* indices, u32 triangleCount)
{
	auto ibo = m_objectManager.getBuffer("navMeshVertexIbo");

	auto dst = ibo->map<u16>(vx::gl::Map::Write_Only);
	vx::memcpy(dst.get(), indices, triangleCount * 3);
}

void EditorRenderAspect::updateCamera()
{
	auto projectionMatrix = m_renderContext.getProjectionMatrix();

	Camerablock block;
	m_camera.getViewMatrix(&block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.cameraPosition = m_camera.getPosition();

	m_cameraBuffer.subData(0, sizeof(Camerablock), &block);
}

const vx::Camera& EditorRenderAspect::getCamera() const
{
	return m_camera;
}

bool EditorRenderAspect::setSelectedMeshInstance(const Editor::MeshInstance* instance)
{
	if (instance)
	{
		auto cmd = m_sceneRenderer.getDrawCommand(instance->getNameSid());

		if (cmd.count == 0)
		{
			return false;
		}

		m_selectedInstance.cmd = cmd;
	}

	m_selectedInstance.ptr = instance;

	return true;
}

void EditorRenderAspect::setSelectedMeshInstanceTransform(vx::Transform &transform)
{
	if (m_selectedInstance.ptr)
	{
		auto index = m_selectedInstance.cmd.baseInstance;

		m_sceneRenderer.updateTransform(transform, index);
	}
}

bool EditorRenderAspect::setSelectedMeshInstanceMaterial(const Material* material) const
{
	return m_sceneRenderer.setMeshInstanceMaterial(m_selectedInstance.ptr->getNameSid(), material);
}

void EditorRenderAspect::editorAddMeshInstance(const Editor::MeshInstance &instance)
{
	m_sceneRenderer.editorAddMeshInstance(instance.getMeshInstance());
}

bool EditorRenderAspect::removeSelectedMeshInstance()
{
	return m_sceneRenderer.editorRemoveStaticMeshInstance(m_selectedInstance.ptr->getNameSid());
}

void EditorRenderAspect::updateInfluenceCellBuffer(const InfluenceMap &influenceMap)
{
	auto influenceCells = influenceMap.getCells();
	auto cellCount = influenceMap.getCellCount();
	auto triangles = influenceMap.getTriangles();
	printf("InfluenceCellsNew: %u\n", cellCount);

	auto vbo = m_objectManager.getBuffer("drawInfluenceCellNewVbo");
	auto mappedBuffer = vbo->map<VertexNavMesh>(vx::gl::Map::Write_Only);

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

void EditorRenderAspect::updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph)
{
	auto nodes = navMeshGraph.getNodes();
	auto nodeCount = navMeshGraph.getNodeCount();

	auto src = vx::make_unique < vx::float3[]>(nodeCount);
	for (u32 i = 0; i < nodeCount; ++i)
	{
		src[i] = nodes[i].m_position;
	}

	m_navMeshGraphNodesCount = nodeCount;

	auto navMeshGraphNodesVbo = m_objectManager.getBuffer("navMeshGraphNodesVbo");
	auto mappedBuffer = navMeshGraphNodesVbo->map<vx::float3>(vx::gl::Map::Write_Only);
	vx::memcpy(mappedBuffer.get(), src.get(), nodeCount);

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

void EditorRenderAspect::updateLightBuffer(const Light* lights, u32 count)
{
	m_sceneRenderer.updateLights(lights, count);

	{
		auto cmdBuffer = m_objectManager.getBuffer("lightCmdBuffer");
		auto mappedCmdBuffer = cmdBuffer->map<vx::gl::DrawArraysIndirectCommand>(vx::gl::Map::Write_Only);
		mappedCmdBuffer->count = count;
	}
}

void EditorRenderAspect::showNavmesh(bool b)
{
	if (b)
	{
		m_commandList.enableSegment("segmentDrawNavmesh.txt");
	}
	else
	{
		m_commandList.disableSegment("segmentDrawNavmesh.txt");
	}
}

void EditorRenderAspect::showInfluenceMap(bool b)
{
	if (b)
	{
		m_commandList.enableSegment("segmentDrawInfluenceCell.txt");
	}
	else
	{
		m_commandList.disableSegment("segmentDrawInfluenceCell.txt");

	}
}