#include "EditorRenderAspect.h"
#include "Event.h"
#include "EventTypes.h"
#include "Scene.h"
#include <vxLib/gl/gl.h>
#include "UniformBlocks.h"
#include <vxLib/gl/StateManager.h>

enum class EditorRenderAspect::EditorUpdate : U32{ Update_None, Update_Mesh, Update_Material, Editor_Added_Instance, Editor_Update_Instance, Editor_Set_Scene };

EditorRenderAspect::EditorRenderAspect(Logfile &logfile, FileAspect &fileAspect)
	:RenderAspect(logfile, fileAspect)
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

	m_editorData.initialize();

	return initializeImpl(dataDir, windowResolution, debug, 0.0f, pAllocator, nullptr, nullptr);
}

void EditorRenderAspect::update()
{
	if (m_updateEditor.load() != 0)
	{
		updateEditor();
		m_updateEditor.store(0);
	}
}

void EditorRenderAspect::render()
{
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cameraBuffer.getId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lightDataBlock.getId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 2, m_cameraBufferStatic.getId());
	glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_gbufferBlock.getId());

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_transformBlock.getId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_materialBlock.getId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_textureBlock.getId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, m_commandBlock.getId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_bvhBlock.getId());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, m_meshVertexBlock.getId());

	glClearTexImage(m_rayTraceShadowTexture.getId(), 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	vx::gl::StateManager::setClearColor(0, 0, 0, 0);
	{
		//m_stageForwardRender.draw(m_stateManager);

		vx::gl::StateManager::enable(vx::gl::Capabilities::Depth_Test);

		vx::gl::StateManager::bindFrameBuffer(m_gbufferFB.getId());
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto pPipeline = m_shaderManager.getPipeline("forward_render.pipe");
		vx::gl::StateManager::bindPipeline(pPipeline->getId());

		vx::gl::StateManager::bindVertexArray(m_meshVao.getId());

		m_commandBlock.bind();
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, 0, m_meshInstancesCountTotal, sizeof(vx::gl::DrawElementsIndirectCommand));

		//m_stageForwardRender = RenderStage(, m_meshVao, m_commandBlock, 0, m_pColdData->m_windowResolution, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, std::move(forwardFramebuffer));

		vx::gl::StateManager::disable(vx::gl::Capabilities::Depth_Test);
	}

	{
		vx::gl::StateManager::setClearColor(0.1f, 0.1f, 0.1f, 1);

		vx::gl::StateManager::bindFrameBuffer(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// only do ray tracing if we actually have stuff
		if (m_meshInstancesCountTotal != 0)
		{
			vx::gl::StateManager::bindVertexArray(m_emptyVao.getId());

			auto pPipeline = m_shaderManager.getPipeline("ray_trace_draw_editor.pipe");
			vx::gl::StateManager::bindPipeline(pPipeline->getId());
			glDrawArrays(GL_POINTS, 0, 1);

		}

		m_editorData.drawMouse(m_shaderManager);
		m_editorData.drawWaypoints(m_shaderManager);
	}

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

void EditorRenderAspect::editor_addMesh(const vx::StringID64 &sid, const char* name, const vx::Mesh* pMesh)
{
	VX_UNREFERENCED_PARAMETER(sid);
	VX_UNREFERENCED_PARAMETER(name);
	VX_UNREFERENCED_PARAMETER(pMesh);
	/*std::lock_guard<std::mutex> guard(m_updateDataMutex);
	if (m_pCurrentScene->addMesh(sid, name, pMesh) != 0)
	{
	m_updateData.push_back(std::make_pair(sid, EditorUpdate::Update_Mesh));

	m_updateEditor.store(1);
	}*/
}

void EditorRenderAspect::editor_addMaterial(const vx::StringID64 &sid, const char* name, Material* pMaterial)
{
	VX_UNREFERENCED_PARAMETER(name);
	VX_ASSERT(sid != 0u);
	VX_ASSERT(pMaterial);
	/*std::lock_guard<std::mutex> guard(m_updateDataMutex);
	if (m_pCurrentScene->addMaterial(sid, name, pMaterial) != 0)
	{
	m_updateData.push_back(std::make_pair(sid, EditorUpdate::Update_Material));

	m_updateEditor.store(1);
	}*/
}

U8 EditorRenderAspect::editor_addMeshInstance(const vx::StringID64 instanceSid, const vx::StringID64 meshSid, const vx::StringID64 materialSid, const vx::Transform &transform)
{
	VX_UNREFERENCED_PARAMETER(instanceSid);
	VX_UNREFERENCED_PARAMETER(meshSid);
	VX_UNREFERENCED_PARAMETER(materialSid);
	VX_UNREFERENCED_PARAMETER(transform);
	/*std::lock_guard<std::mutex> guard(m_updateDataMutex);

	if (m_pCurrentScene->addMeshInstance(instanceSid, meshSid, materialSid, transform) != 0)
	{
	m_updateData.push_back(std::make_pair(vx::StringID64(), EditorUpdate::Editor_Added_Instance));

	m_updateEditor.store(1);

	return 1;
	}*/

	return 0;
}

U32 EditorRenderAspect::editor_getTransform(const vx::StringID64 instanceSid, vx::float3 &translation, vx::float3 &rotation, F32 &scaling)
{
	VX_UNREFERENCED_PARAMETER(instanceSid);
	VX_UNREFERENCED_PARAMETER(translation);
	VX_UNREFERENCED_PARAMETER(rotation);
	VX_UNREFERENCED_PARAMETER(scaling);
	/*auto p = m_pCurrentScene->findMeshInstance(instanceSid);
	if (p)
	{
	auto transform = p->getTransform();
	translation = transform.m_translation;
	rotation = transform.m_rotation;
	scaling = transform.m_scaling;
	return 1;
	}*/

	return 0;
}

void EditorRenderAspect::editor_updateTranslation(const vx::StringID64 instanceSid, const vx::float3 &translation)
{
	VX_UNREFERENCED_PARAMETER(instanceSid);
	VX_UNREFERENCED_PARAMETER(translation);
	/*std::lock_guard<std::mutex> guard(m_updateDataMutex);

	auto p = m_pCurrentScene->findMeshInstance(instanceSid);
	if (p)
	{
	p->setTranslation(translation);
	m_updateData.push_back(std::make_pair(instanceSid, EditorUpdate::Editor_Update_Instance));

	m_updateEditor.store(1);
	}*/
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