#include "EditorRenderAspect.h"
#include "Event.h"
#include "EventTypes.h"
#include "Scene.h"
#include <vxLib/gl/gl.h>
#include <vxLib/gl/StateManager.h>
#include <vxLib/gl/ProgramPipeline.h>
#include <vxLib/util/DebugPrint.h>
#include "developer.h"
#include "GpuStructs.h"
#include "InfluenceMap.h"
#include "NavMeshGraph.h"
#include "utility.h"

struct VertexNavMesh
{
	vx::float3 position;
	vx::float3 color;
};

struct InfluenceCellVertex
{
	vx::float3 position;
	U32 count;
};

enum class EditorRenderAspect::EditorUpdate : U32{ Update_None, Update_Mesh, Update_Material, Editor_Added_Instance, Editor_Update_Instance, Editor_Set_Scene };

EditorRenderAspect::EditorRenderAspect()
	:RenderAspect()
{
}

bool EditorRenderAspect::initialize(const std::string &dataDir, HWND panel, HWND tmp, const vx::uint2 &windowResolution,
	F32 fovDeg, F32 zNear, F32 zFar, bool vsync, bool debug, vx::StackAllocator *pAllocator)
{
	if (!m_renderContext.initializeExtensions(tmp))
	{
		puts("Error initializing Extensions");
		return false;
	}

	vx::gl::OpenGLDescription glDesc = vx::gl::OpenGLDescription::create(panel, windowResolution, vx::degToRad(fovDeg), zNear, zFar, 4, 5, vsync, debug);
	if (!m_renderContext.initializeOpenGl(glDesc))
	{
		puts("Error initializing Context");
		return false;
	}

	m_pEditorColdData = std::make_unique<EditorColdData>();
	m_editorData.initialize();

	createNavMeshVertexBuffer();
	createNavMeshIndexBuffer();
	createNavMeshVertexVao();
	createNavMeshVao();

	createInfluenceCellVbo();
	createInfluenceCellVao();

	createNavMeshNodesVbo();
	createNavMeshNodesVao();

	createIndirectCmdBuffers();

	auto result = initializeImpl(dataDir, windowResolution, debug, pAllocator);

	bindBuffers();

	return result;
}

void EditorRenderAspect::createNavMeshVertexBuffer()
{
	vx::gl::BufferDescription dsc;
	dsc.bufferType = vx::gl::BufferType::Array_Buffer;
	dsc.flags = vx::gl::BufferStorageFlags::Write;
	dsc.immutable = 1;
	dsc.size = sizeof(VertexNavMesh) * 256;
	m_pEditorColdData->m_navMeshVertexVbo.create(dsc);
}

void EditorRenderAspect::createNavMeshIndexBuffer()
{
	vx::gl::BufferDescription dsc;
	dsc.bufferType = vx::gl::BufferType::Element_Array_Buffer;
	dsc.flags = vx::gl::BufferStorageFlags::Write;
	dsc.immutable = 1;
	dsc.size = sizeof(U16) * 256 * 3;
	m_pEditorColdData->m_navMeshVertexIbo.create(dsc);
}

void EditorRenderAspect::createNavMeshVao()
{
	m_pEditorColdData->m_navMeshVao.create();
	m_pEditorColdData->m_navMeshVao.enableArrayAttrib(0);
	m_pEditorColdData->m_navMeshVao.arrayAttribFormatF(0, 3, 0, 0);
	m_pEditorColdData->m_navMeshVao.arrayAttribBinding(0, 0);
	m_pEditorColdData->m_navMeshVao.bindVertexBuffer(m_pEditorColdData->m_navMeshVertexVbo, 0, 0, sizeof(VertexNavMesh));

	m_pEditorColdData->m_navMeshVao.bindIndexBuffer(m_pEditorColdData->m_navMeshVertexIbo);
}

void EditorRenderAspect::createNavMeshVertexVao()
{
	m_pEditorColdData->m_navMeshVertexVao.create();
	m_pEditorColdData->m_navMeshVertexVao.enableArrayAttrib(0);
	m_pEditorColdData->m_navMeshVertexVao.arrayAttribFormatF(0, 3, 0, 0);
	m_pEditorColdData->m_navMeshVertexVao.arrayAttribBinding(0, 0);

	m_pEditorColdData->m_navMeshVertexVao.enableArrayAttrib(1);
	m_pEditorColdData->m_navMeshVertexVao.arrayAttribFormatF(1, 3, 0, sizeof(vx::float3));
	m_pEditorColdData->m_navMeshVertexVao.arrayAttribBinding(1, 0);

	m_pEditorColdData->m_navMeshVertexVao.bindVertexBuffer(m_pEditorColdData->m_navMeshVertexVbo, 0, 0, sizeof(VertexNavMesh));
}

void EditorRenderAspect::createInfluenceCellVbo()
{
	vx::gl::BufferDescription dsc;
	dsc.bufferType = vx::gl::BufferType::Array_Buffer;
	dsc.flags = vx::gl::BufferStorageFlags::Write;
	dsc.immutable = 1;
	dsc.size = sizeof(InfluenceCellVertex) * 256;
	m_pEditorColdData->m_influenceCellVbo.create(dsc);
}

void EditorRenderAspect::createInfluenceCellVao()
{
	m_pEditorColdData->m_influenceVao.create();

	m_pEditorColdData->m_influenceVao.enableArrayAttrib(0);
	m_pEditorColdData->m_influenceVao.arrayAttribFormatF(0, 3, 0, 0);
	m_pEditorColdData->m_influenceVao.arrayAttribBinding(0, 0);

	m_pEditorColdData->m_influenceVao.enableArrayAttrib(1);
	m_pEditorColdData->m_influenceVao.arrayAttribFormatI(1, 1, vx::gl::DataType::Unsigned_Int, sizeof(vx::float3));
	m_pEditorColdData->m_influenceVao.arrayAttribBinding(1, 0);

	m_pEditorColdData->m_influenceVao.bindVertexBuffer(m_pEditorColdData->m_influenceCellVbo, 0, 0, sizeof(InfluenceCellVertex));
}

void EditorRenderAspect::createNavMeshNodesVbo()
{
	vx::gl::BufferDescription dsc;
	dsc.bufferType = vx::gl::BufferType::Array_Buffer;
	dsc.flags = vx::gl::BufferStorageFlags::Write;
	dsc.immutable = 1;
	dsc.size = sizeof(vx::float3) * 256;
	m_pEditorColdData->m_navMeshGraphNodesVbo.create(dsc);
}

void EditorRenderAspect::createNavMeshNodesVao()
{
	m_navMeshGraphNodesVao.create();

	m_navMeshGraphNodesVao.enableArrayAttrib(0);
	m_navMeshGraphNodesVao.arrayAttribFormatF(0, 3, 0, 0);
	m_navMeshGraphNodesVao.arrayAttribBinding(0, 0);

	m_navMeshGraphNodesVao.bindVertexBuffer(m_pEditorColdData->m_navMeshGraphNodesVbo, 0, 0, sizeof(vx::float3));
}

void EditorRenderAspect::createIndirectCmdBuffers()
{
	DrawArraysIndirectCommand arrayCmd;
	memset(&arrayCmd, 0, sizeof(DrawArraysIndirectCommand));
	arrayCmd.instanceCount = 1;

	vx::gl::BufferDescription desc;
	desc.bufferType = vx::gl::BufferType::Draw_Indirect_Buffer;
	desc.flags = vx::gl::BufferStorageFlags::Write;
	desc.immutable = 1;
	desc.pData = &arrayCmd;
	desc.size = sizeof(DrawArraysIndirectCommand);

	m_pEditorColdData->m_navMeshVertexCmdBuffer.create(desc);
	m_pEditorColdData->m_lightCmdBuffer.create(desc);
	m_pEditorColdData->m_influenceMapCmdBuffer.create(desc);

	vx::gl::DrawElementsIndirectCommand elementsCmd;
	memset(&elementsCmd, 0, sizeof(vx::gl::DrawElementsIndirectCommand));

	desc.pData = &elementsCmd;
	desc.size = sizeof(vx::gl::DrawElementsIndirectCommand);
	elementsCmd.instanceCount = 1;

	m_pEditorColdData->m_navmeshCmdBuffer.create(desc);
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

/*void EditorRenderAspect::renderLights()
{
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

	auto pipe = m_shaderManager.getPipeline("editorDrawLights.pipe");
	vx::gl::StateManager::bindPipeline(*pipe);

	vx::gl::StateManager::bindVertexArray(m_emptyVao);

	glPointSize(5.0f);

	glDrawArrays(GL_POINTS, 0, m_lightCount);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}*/

/*void EditorRenderAspect::renderNavMeshVertices()
{
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

	auto pipe = m_shaderManager.getPipeline("editorDrawPointColor.pipe");
	auto fsShader = pipe->getFragmentShader();
	vx::gl::StateManager::bindPipeline(*pipe);

	vx::gl::StateManager::bindVertexArray(m_navMeshVertexVao);
	glPointSize(5.0f);

	glDrawArrays(GL_POINTS, 0, m_navMeshVertexCount);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}*/

void EditorRenderAspect::renderNavMeshGraphNodes()
{
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);

	auto pipe = m_shaderManager.getPipeline("editorDrawPoint.pipe");
	auto fsShader = pipe->getFragmentShader();

	const F32 color[] = {0, 1, 0, 0.5f};

	glProgramUniform4fv(fsShader, 0, 1, color);

	vx::gl::StateManager::bindPipeline(*pipe);

	vx::gl::StateManager::bindVertexArray(m_navMeshGraphNodesVao);
	glPointSize(5.0f);

	glDrawArrays(GL_POINTS, 0, m_navMeshGraphNodesCount);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}

/*void EditorRenderAspect::renderNavMesh()
{
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto pPipeline = m_shaderManager.getPipeline("navmesh.pipe");
	vx::float4 color(0.5f, 0, 0.5f, 0.25f);
	glProgramUniform4fv(pPipeline->getFragmentShader(), 0, 1, color);

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_navMeshVao.getId());
	glDrawElements(GL_TRIANGLES, m_navMeshIndexCount, GL_UNSIGNED_SHORT, 0);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);
}

void EditorRenderAspect::renderInfluenceMap()
{
	vx::gl::StateManager::enable(vx::gl::Capabilities::Blend);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	auto pPipeline = m_shaderManager.getPipeline("influenceMap.pipe");

	vx::gl::StateManager::bindPipeline(pPipeline->getId());
	vx::gl::StateManager::bindVertexArray(m_influenceVao.getId());
	glDrawArrays(GL_POINTS, 0, m_influenceCellCount);

	vx::gl::StateManager::disable(vx::gl::Capabilities::Blend);
}*/

void EditorRenderAspect::render()
{
	clearTextures();
	clearBuffers();

	vx::gl::StateManager::bindFrameBuffer(0);
	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto &cmdBuffer = m_sceneRenderer.getCmdBuffer();
	auto meshCount = m_sceneRenderer.getMeshInstanceCount();
	auto &meshVao = m_sceneRenderer.getMeshVao();

	vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

	vx::gl::StateManager::bindVertexArray(meshVao);

	cmdBuffer.bind();

	if (m_selectedInstance.ptr != nullptr)
	{
		auto offset = m_selectedInstance.cmd.baseInstance * sizeof(vx::gl::DrawElementsIndirectCommand);

		auto pipe = m_shaderManager.getPipeline("editorSelectedMesh.pipe");
		vx::gl::StateManager::bindPipeline(*pipe);

		glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)offset);
	}

	auto pipe = m_shaderManager.getPipeline("editor.pipe");
	vx::gl::StateManager::bindPipeline(*pipe);

	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, meshCount, sizeof(vx::gl::DrawElementsIndirectCommand));

//	m_commandList.draw();

	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	m_renderContext.swapBuffers();
}

void EditorRenderAspect::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case(EventType::File_Event) :
		handleFileEvent(evt);
		break;
	case(EventType::Ingame_Event) :
		handleIngameEvent(evt);
		break;
	case(EventType::Editor_Event) :
		handleEditorEvent(evt);
		break;
	default:
		break;
	}
}

void EditorRenderAspect::addMesh(const vx::StringID64 &sid)
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

void EditorRenderAspect::addMaterial(const vx::StringID64 &sid)
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

	U32 batchIndexStart = 0;
	U32 batchInstanceCount = 0;
	U32 batchInstanceStart = 0;
	U32 totalInstanceCount = 0;

	m_editorMeshInstanceIndices.clear();

	std::vector<std::pair<vx::StringID64, const MeshInstance*>> meshInstances;
	meshInstances.reserve(instanceCount);

	auto &sceneMeshInstances = m_pCurrentScene->getMeshInstancesSortedByName();
	for (auto i = 0u; i < instanceCount; ++i)
	{
	meshInstances.push_back(
	std::make_pair(sceneMeshInstances.keys()[i], sceneMeshInstances.data() + i)
	);
	}

	auto &sceneMaterials = m_pCurrentScene->getMaterials();
	std::sort(meshInstances.begin(), meshInstances.end(), [&](const std::pair<vx::StringID64, const MeshInstance*> &lhs, const std::pair<vx::StringID64, const MeshInstance*> &rhs)
	{
	Material &lhsMaterial = **sceneMaterials.find(lhs.second->getMaterialSid());
	Material &rhsMaterial = **sceneMaterials.find(rhs.second->getMaterialSid());

	return (lhsMaterial < rhsMaterial) ||
	(lhsMaterial == rhsMaterial && lhs.second->getMeshSid() < rhs.second->getMeshSid());
	});

	auto batchMeshSid = meshInstances[0].second->getMeshSid();

	U32 drawCount = 0;
	U32 batchIndexCount = m_editorMeshEntries.find(batchMeshSid)->indexCount;

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

void EditorRenderAspect::updateInstance(const vx::StringID64 &sid)
{
	VX_UNREFERENCED_PARAMETER(sid);
	/*auto pInstance = m_pCurrentScene->findMeshInstance(sid);

	U32 id = 0;
	auto found = m_editorData.getMeshInstanceId(sid, &id);
	if (found && pInstance)
	{
	//writeMeshInstanceTransform(pInstance, *it);
	}*/
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

void EditorRenderAspect::editor_moveCamera(F32 dirX, F32 dirY, F32 dirZ)
{
	const F32 speed = 0.05f;

	__m128 direction = { dirX, dirY, dirZ, 0 };
	m_camera.move(direction, speed);
}

void VX_CALLCONV EditorRenderAspect::editor_rotateCamera(const __m128 rotation)
{
	m_camera.setRotation(rotation);
}

void EditorRenderAspect::editor_updateMouseHit(const vx::float3 &p)
{
	m_editorData.updateMouseHit(p);
}

void EditorRenderAspect::editor_updateWaypoint(U32 offset, U32 count, const Waypoint* src)
{
	m_editorData.updateWaypoint(offset, count, src);
}

void EditorRenderAspect::handleEditorEvent(const Event &evt)
{
	VX_UNREFERENCED_PARAMETER(evt);
}

void EditorRenderAspect::handleLoadScene(const Event &evt)
{
	auto scene = (Scene*)evt.arg1.ptr;
	m_pEditorColdData->m_lightCount = scene->getLightCount();
	auto &navMesh = scene->getNavMesh();

	updateNavMeshBuffer(navMesh);
}

void EditorRenderAspect::handleFileEvent(const Event &evt)
{
	RenderAspect::handleFileEvent(evt);

	if ((FileEvent)evt.code == FileEvent::Scene_Loaded)
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
		U32 tmp[3];
		updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, tmp, 0);
	}

	updateNavMeshIndexBuffer(navMesh);
}

void EditorRenderAspect::updateNavMeshBuffer(const NavMesh &navMesh, U32(&selectedVertexIndex)[3], U8 selectedCount)
{
	auto navMeshVertexCount = navMesh.getVertexCount();
	if (navMeshVertexCount != 0)
	{
		auto vertices = navMesh.getVertices();
		updateNavMeshVertexBufferWithSelectedVertex(vertices, navMeshVertexCount, selectedVertexIndex, selectedCount);
	}

	updateNavMeshIndexBuffer(navMesh);
}

void EditorRenderAspect::uploadToNavMeshVertexBuffer(const VertexNavMesh* vertices, U32 count)
{
	auto dst = m_pEditorColdData->m_navMeshVertexVbo.map<VertexNavMesh>(vx::gl::Map::Write_Only);
	::memcpy(dst.get(), vertices, sizeof(VertexNavMesh) * count);
}

void EditorRenderAspect::updateNavMeshVertexBufferWithSelectedVertex(const vx::float3* vertices, U32 count, U32(&selectedVertexIndex)[3], U8 selectedCount)
{
	auto color = vx::float3(1, 0, 0);
	auto src = std::make_unique<VertexNavMesh[]>(count);
	for (U32 i = 0; i < count; ++i)
	{
		src[i].position = vertices[i];
		src[i].color = color;
	}

	for (U8 i = 0; i < selectedCount; ++i)
	{
		auto index = selectedVertexIndex[i];
		src[index].color.y = 1.0f;
	}

	m_pEditorColdData->m_navMeshVertexCount = count;
	uploadToNavMeshVertexBuffer(src.get(), count);
}

void EditorRenderAspect::updateNavMeshIndexBuffer(const NavMesh &navMesh)
{
	auto triangleCount = navMesh.getTriangleCount();
	if (triangleCount != 0)
	{
		auto indices = navMesh.getTriangleIndices();
		updateNavMeshIndexBuffer(indices, triangleCount);

		m_pEditorColdData->m_navMeshIndexCount = triangleCount * 3;
	}
}

void EditorRenderAspect::updateNavMeshIndexBuffer(const TriangleIndices* indices, U32 triangleCount)
{
	auto dst = m_pEditorColdData->m_navMeshVertexIbo.map<U16>(vx::gl::Map::Write_Only);
	::memcpy(dst.get(), indices, sizeof(TriangleIndices) * triangleCount);
}

void EditorRenderAspect::updateCamera()
{
	auto projectionMatrix = m_renderContext.getProjectionMatrix();

	Camerablock block;
	m_camera.getViewMatrix(block.viewMatrix);
	block.pvMatrix = projectionMatrix * block.viewMatrix;
	block.inversePVMatrix = vx::MatrixInverse(block.pvMatrix);
	block.cameraPosition = m_camera.getPosition();

	m_cameraBuffer.subData(0, sizeof(Camerablock), &block);
}

const vx::Camera& EditorRenderAspect::getCamera() const
{
	return m_camera;
}

bool EditorRenderAspect::setSelectedMeshInstance(const MeshInstance* p)
{
	if (p)
	{
		auto cmd = m_sceneRenderer.getDrawCommand(p);

		if (cmd.count == 0)
		{
			return false;
		}

		m_selectedInstance.cmd = cmd;
	}

	m_selectedInstance.ptr = p;

	return true;
}

void EditorRenderAspect::updateSelectedMeshInstanceTransform(vx::Transform &transform)
{
	if (m_selectedInstance.ptr)
	{
		auto index = m_selectedInstance.cmd.baseInstance;

		m_sceneRenderer.updateTransform(transform, index);
	}
}

void EditorRenderAspect::updateInfluenceCellBuffer(const InfluenceMap &influenceMap)
{
	auto cellCount = influenceMap.getCellCount();
	auto cells = influenceMap.getInfluenceCells();

	auto vertices = std::make_unique<InfluenceCellVertex[]>(cellCount);
	for (U32 i = 0; i < cellCount; ++i)
	{
		vertices[i].position = cells[i].m_position;
		vertices[i].count = cells[i].m_count;
	}

	m_pEditorColdData->m_influenceCellCount = cellCount;

	auto mappedBuffer = m_pEditorColdData->m_influenceCellVbo.map<InfluenceCellVertex>(vx::gl::Map::Write_Only);
	::memcpy(mappedBuffer.get(), vertices.get(), sizeof(InfluenceCellVertex) * cellCount);
}

void EditorRenderAspect::updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph)
{
	auto nodes = navMeshGraph.getNodes();
	auto nodeCount = navMeshGraph.getNodeCount();

	auto src = std::make_unique < vx::float3[]>(nodeCount);
	for (U32 i = 0; i < nodeCount; ++i)
	{
		src[i] = nodes[i].position;
	}

	m_navMeshGraphNodesCount = nodeCount;

	auto mappedBuffer = m_pEditorColdData->m_navMeshGraphNodesVbo.map<vx::float3>(vx::gl::Map::Write_Only);
	vx::memcpy(mappedBuffer.get(), src.get(), nodeCount);
}