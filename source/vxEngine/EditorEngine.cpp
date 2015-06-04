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
#include "EditorEngine.h"
#include "enums.h"
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include "developer.h"
#include <vxLib/util/DebugPrint.h>
#include "Locator.h"
#include "MeshInstance.h"
#include "Ray.h"
#include "EditorScene.h"
#include "NavMeshGraph.h"
#include "Light.h"
#include "EngineConfig.h"

u32 EditorEngine::s_editorTypeMesh{ 0xffffffff };
u32 EditorEngine::s_editorTypeMaterial{ 0xffffffff };
u32 EditorEngine::s_editorTypeScene{ 0xffffffff };

EditorEngine::EditorEngine()
	:m_eventManager(),
	m_physicsAspect(m_fileAspect),
	m_renderAspect(),
	m_fileAspect(),
	m_bRunFileThread(),
	m_fileAspectThread()
{
}

EditorEngine::~EditorEngine()
{
	if (m_shutdown == 0)
	{
		// something bad happened
		assert(false);
	}
}

void EditorEngine::loopFileThread()
{
	while (m_bRunFileThread.load() != 0)
	{
		// do work
		m_fileAspect.update();

		// sleep for a bit
		std::this_thread::yield();
	}
}

bool EditorEngine::initializeImpl(const std::string &dataDir)
{
	m_memory = Memory(100 MBYTE, 64);

	m_allocator = vx::StackAllocator(m_memory.get(), m_memory.size());

	m_scratchAllocator = vx::StackAllocator(m_allocator.allocate(1 MBYTE, 64), 1 MBYTE);

	if (!m_fileAspect.initialize(&m_allocator, dataDir))
		return false;

	Locator::provide(&m_fileAspect);

	return true;
}

void EditorEngine::createStateMachine()
{
}

bool EditorEngine::initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, EditorScene* pScene)
{
	vx::activateChannel(dev::Channel_Render);
	vx::activateChannel(dev::Channel_Editor);
	vx::activateChannel(dev::Channel_FileAspect);
	vx::DebugPrint::g_verbosity = 1;

	const std::string dataDir("../../game/data/");
	m_pEditorScene = pScene;
	m_resolution = resolution;
	m_panel = panel;

	if (!initializeImpl(dataDir))
		return false;

	g_engineConfig.m_fov = 66.0f;
	g_engineConfig.m_zFar = 1000.0f;
	g_engineConfig.m_renderDebug = true;
	g_engineConfig.m_resolution = resolution;
	g_engineConfig.m_vsync = false;
	g_engineConfig.m_zNear = 0.1f;

	if (!m_renderAspect.initialize(dataDir, panel, tmp, &m_allocator, &g_engineConfig))
	{
		puts("Error initializing Renderer");
		return false;
	}

	dev::g_debugRenderSettings.setVoxelize(0);
	dev::g_debugRenderSettings.setShadingMode(ShadingMode::Albedo);
	RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::ToggleRenderMode;
	m_renderAspect.queueUpdateTask(task);

	if (!m_physicsAspect.initialize())
	{
		return false;
	}

	Locator::provide(&m_physicsAspect);

	m_eventManager.initialize();
	Locator::provide(&m_eventManager);

	m_eventManager.registerListener(&m_renderAspect, 1, (u8)vx::EventType::File_Event);
	m_eventManager.registerListener(&m_physicsAspect, 1, (u8)vx::EventType::File_Event);
	m_eventManager.registerListener(this, 1, (u8)vx::EventType::File_Event);

	//m_bRun = 1;
	m_bRunFileThread.store(1);
	m_shutdown = 0;

	createStateMachine();

	memset(&m_selected, 0, sizeof(m_selected));

	return true;
}

void EditorEngine::shutdownEditor()
{
	m_fileAspectThread.join();
	m_fileAspect.shutdown();

	m_physicsAspect.shutdown();
	m_renderAspect.shutdown(m_panel);
	m_panel = nullptr;

	m_shutdown = 1;
	m_allocator.release();
}

void EditorEngine::stop()
{
	m_bRunFileThread.store(0);
}

void EditorEngine::buildNavGraph()
{
	auto &navMesh = m_pEditorScene->getNavMesh();

	m_influenceMap.initialize(navMesh);

	NavMeshGraph graph;
	graph.initialize(navMesh);

	m_renderAspect.updateInfluenceCellBuffer(m_influenceMap);
	m_renderAspect.updateNavMeshGraphNodesBuffer(graph);
}

void EditorEngine::handleEvent(const vx::Event &evt)
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

void EditorEngine::handleFileEvent(const vx::Event &evt)
{
	auto fe = (vx::FileEvent)evt.code;

	switch (fe)
	{
	case vx::FileEvent::Mesh_Loaded:
	{
		//vx::verboseChannelPrintF(1, dev::Channel_Editor, "Loaded Mesh");
		/*std::string* pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);
		auto sid = evt.arg1.sid;

		auto pMesh = m_fileAspect.getMesh(sid);
		//m_renderAspect.editor_addMesh(sid, pStr->c_str(), pMesh);

		call_editorCallback(sid);

		delete(pStr);*/
	}break;
	case vx::FileEvent::Texture_Loaded:
		break;
	case vx::FileEvent::Material_Loaded:
	{
		//vx::verboseChannelPrintF(1, dev::Channel_Editor, "Loaded Material");
		//std::string* pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);

		//auto pMaterial = m_fileAspect.getMaterial(sid);
		//VX_ASSERT(pMaterial != nullptr);
		//m_renderAspect.editor_addMaterial(sid, pStr->c_str(), pMaterial);

		//call_editorCallback(vx::StringID(evt.arg1.u64));

		//delete(pStr);
	}break;
	case vx::FileEvent::Scene_Loaded:
		vx::verboseChannelPrintF(0, dev::Channel_Editor, "Loaded Scene");
		call_editorCallback(vx::StringID(evt.arg2.u64));

		buildNavGraph();
		break;
	default:
		break;
	}
}

void EditorEngine::requestLoadFile(const FileEntry &fileEntry, void* p)
{
	m_fileAspect.requestLoadFile(fileEntry, p);
}

void EditorEngine::editor_saveScene(const char* name)
{
	m_fileAspect.requestSaveFile(FileEntry(name, FileType::Scene), m_pEditorScene);
}

void EditorEngine::editor_setTypes(u32 mesh, u32 material, u32 scene)
{
	s_editorTypeMesh = mesh;
	s_editorTypeMaterial = material;
	s_editorTypeScene = scene;
}

void EditorEngine::editor_start()
{
	m_fileAspectThread = vx::thread(&EditorEngine::loopFileThread, this);
}

void EditorEngine::editor_render()
{
	m_eventManager.update();

	m_renderAspect.update();

	m_renderAspect.render();
}

void EditorEngine::editor_loadFile(const char *filename, u32 type, Editor::LoadFileCallback f)
{
	assert(s_editorTypeMesh != s_editorTypeMaterial);

	void *p = nullptr;
	FileEntry fileEntry;
	if (type == s_editorTypeMesh)
	{
		//fileEntry = FileEntry(filename, FileType::Mesh);
		//p = new std::string(filename);
		assert(false);
	}
	else if (type == s_editorTypeMaterial)
	{
		//fileEntry = FileEntry(filename, FileType::Material);
		//	p = new std::string(filename);
		assert(false);
	}
	else if (type == s_editorTypeScene)
	{
		fileEntry = FileEntry(filename, FileType::Scene);
		p = m_pEditorScene;
	}
	else
	{
		assert(false);
	}

	std::lock_guard<std::mutex> guard(m_editorMutex);
	m_requestedFiles.insert(vx::make_sid(filename), std::make_pair(f, type));

	m_fileAspect.requestLoadFile(fileEntry, p);
}

void EditorEngine::editor_moveCamera(f32 dirX, f32 dirY, f32 dirZ)
{
	m_renderAspect.editor_moveCamera(dirX, dirY, dirZ);
}

void EditorEngine::editor_rotateCamera(f32 dirX, f32 dirY, f32 dirZ)
{
	static f32 x = 0.0f;
	static f32 y = 0.0f;

	x += dirX * 0.01f;
	y += dirY * 0.01f;

	vx::float4a rot(y, x, 0, 0);
	auto v = vx::quaternionRotationRollPitchYawFromVector(rot);

	m_renderAspect.editor_rotateCamera(v);
}

void EditorEngine::call_editorCallback(const vx::StringID &sid)
{
	std::lock_guard<std::mutex> guard(m_editorMutex);
	auto it = m_requestedFiles.find(sid);
	if (it != m_requestedFiles.end())
	{
		(*it->first)(sid.value, it->second);
		m_requestedFiles.erase(it);
	}
}

vx::float4a EditorEngine::getRayDir(s32 mouseX, s32 mouseY)
{
	f32 ndc_x = f32(mouseX) / m_resolution.x;
	f32 ndc_y = f32(mouseY) / m_resolution.y;

	ndc_x = ndc_x * 2.0f - 1.0f;
	ndc_y = 1.0f - ndc_y * 2.0f;

	vx::mat4 projMatrix;
	m_renderAspect.getProjectionMatrix(&projMatrix);
	projMatrix = vx::MatrixInverse(projMatrix);

	vx::float4a ray_clip(ndc_x, ndc_y, -1, 1);
	vx::float4a ray_eye = vx::Vector4Transform(projMatrix, ray_clip);
	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	vx::mat4 viewMatrix;
	m_renderAspect.getCamera().getViewMatrix(&viewMatrix);
	auto inverseViewMatrix = vx::MatrixInverse(viewMatrix);
	vx::float4a ray_world = vx::Vector4Transform(inverseViewMatrix, ray_eye);
	ray_world = vx::normalize3(ray_world);

	return ray_world;
}

MeshInstance* EditorEngine::raytraceAgainstStaticMeshes(s32 mouseX, s32 mouseY, vx::float3* hitPosition)
{
	auto ray_world = getRayDir(mouseX, mouseY);

	auto cameraPosition = m_renderAspect.getCamera().getPosition();

	vx::float4a tmp = cameraPosition;

	return m_physicsAspect.raycast_static(tmp, ray_world, 50.0f, hitPosition);
}

u32 EditorEngine::getMeshInstanceCount() const
{
	u32 result = 0;

	if (m_pEditorScene)
	{
		result = m_pEditorScene->getMeshInstanceCount();
	}

	return result;
}

const char* EditorEngine::getMeshInstanceName(u32 i) const
{
	if (m_pEditorScene)
	{
		auto meshInstances = m_pEditorScene->getMeshInstances();
		return m_pEditorScene->getMeshInstanceName(meshInstances[i].getNameSid());
	}

	return nullptr;
}

u64 EditorEngine::getMeshInstanceSid(u32 i) const
{
	u64 sidValue = 0;

	if (m_pEditorScene)
	{
		auto meshInstances = m_pEditorScene->getMeshInstances();
		sidValue = meshInstances[i].getNameSid().value;
	}

	return sidValue;
}

const char* EditorEngine::getSelectedMeshInstanceName() const
{
	auto meshInstance = (MeshInstance*)m_selected.m_item;
	const char* name = nullptr;
	if (meshInstance)
	{
		name = m_pEditorScene->getMeshInstanceName(meshInstance->getNameSid());
	}
	return name;
}

u64 EditorEngine::getSelectedMeshInstanceSid() const
{
	u64 sidValue = 0;

	auto meshInstance = (MeshInstance*)m_selected.m_item;
	if (meshInstance)
	{
		sidValue = meshInstance->getNameSid().value;
	}

	return sidValue;
}

u64 EditorEngine::getSelectedMeshInstanceMeshSid() const
{
	u64 sidValue = 0;

	auto meshInstance = (MeshInstance*)m_selected.m_item;
	if (meshInstance)
	{
		sidValue = meshInstance->getMeshSid().value;
	}

	return sidValue;
}

u64 EditorEngine::getSelectedMeshInstanceMaterialSid() const
{
	u64 sidValue = 0;

	auto meshInstance = (MeshInstance*)m_selected.m_item;
	if (meshInstance)
	{
		sidValue = meshInstance->getMaterialSid().value;
	}

	return sidValue;
}

bool EditorEngine::selectMeshInstance(s32 x, s32 y)
{
	bool result = false;
	if (m_pEditorScene)
	{
		vx::float3 p;
		auto ptr = raytraceAgainstStaticMeshes(x, y, &p);
		if (ptr != nullptr)
		{
			m_renderAspect.setSelectedMeshInstance(ptr);
			m_selected.m_type = SelectedType::MeshInstance;
			m_selected.m_item = ptr;

			result = true;
		}
	}

	return result;
}

bool EditorEngine::selectMeshInstance(u32 i)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto instances = m_pEditorScene->getMeshInstances();

		m_renderAspect.setSelectedMeshInstance(&instances[i]);
		m_selected.m_type = SelectedType::MeshInstance;
		m_selected.m_item = (void*)&instances[i];

		result = true;
	}

	return result;
}

void EditorEngine::deselectMeshInstance()
{
	if (m_pEditorScene)
	{
		m_renderAspect.setSelectedMeshInstance(nullptr);
		m_selected.m_item = nullptr;
	}
}

void EditorEngine::setSelectedMeshInstanceMaterial(u64 sid) const
{
	if (m_pEditorScene && m_selected.m_item)
	{
		auto meshInstance = (MeshInstance*)m_selected.m_item;

		auto &sceneMaterials = m_pEditorScene->getMaterials();
		auto it = sceneMaterials.find(vx::StringID(sid));
		if (it != sceneMaterials.end())
		{
			if (m_renderAspect.setSelectedMeshInstanceMaterial(*it))
			{
				meshInstance->setMaterialSid(vx::StringID(sid));
			}
		}
	}
}

void EditorEngine::setSelectedMeshInstanceTransform(const vx::float3 &p)
{
	vx::Transform transform;
	transform.m_translation = p;
	transform.m_rotation = vx::float3(0.0f);
	transform.m_scaling = 1.0f;

	if (m_pEditorScene && m_selected.m_item)
	{
		auto meshInstance = (MeshInstance*)m_selected.m_item;
		meshInstance->setTranslation(transform.m_translation);

		m_renderAspect.setSelectedMeshInstanceTransform(transform);
	}
}

u64 EditorEngine::setSelectedMeshInstanceName(const char* name)
{
	vx::StringID sid;

	if (m_pEditorScene && m_selected.m_item)
	{
		auto meshInstance = (MeshInstance*)m_selected.m_item;

		sid = vx::make_sid(name);

		if (meshInstance->getNameSid() != sid)
		{
			meshInstance->setNameSid(sid);

			m_pEditorScene->addMeshInstanceName(sid, std::string(name));
		}
	}

	return sid.value;
}

bool EditorEngine::addNavMeshVertex(s32 mouseX, s32 mouseY)
{
	vx::float3 hitPos;
	auto ptr = raytraceAgainstStaticMeshes(mouseX, mouseY, &hitPos);
	if (ptr)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.addVertex(hitPos);
		m_renderAspect.updateNavMeshBuffer(navMesh);
	}

	return ptr != nullptr;
}

void EditorEngine::deleteSelectedNavMeshVertex()
{
	if (m_selected.m_navMeshVertices.m_count != 0)
	{
		auto index = m_selected.m_navMeshVertices.m_vertices[0];
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.deleteVertex(index);

		m_renderAspect.updateNavMeshBuffer(navMesh);
	}
}

Ray EditorEngine::getRay(s32 mouseX, s32 mouseY)
{
	auto rayDir = getRayDir(mouseX, mouseY);
	auto cameraPosition = m_renderAspect.getCamera().getPosition();

	Ray ray;
	vx::storeFloat(&ray.o, cameraPosition);
	vx::storeFloat(&ray.d, rayDir);
	ray.maxt = 50.0f;

	return ray;
}

u32 EditorEngine::getSelectedNavMeshVertex(s32 mouseX, s32 mouseY)
{
	auto ray = getRay(mouseX, mouseY);

	auto &navMesh = m_pEditorScene->getNavMesh();

	return navMesh.testRayAgainstVertices(ray);
}

bool EditorEngine::selectNavMeshVertex(s32 mouseX, s32 mouseY)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto selectedIndex = getSelectedNavMeshVertex(mouseX, mouseY);
		if (selectedIndex != 0xffffffff)
		{
			auto &navMesh = m_pEditorScene->getNavMesh();

			m_selected.m_navMeshVertices.m_vertices[0] = selectedIndex;
			m_selected.m_navMeshVertices.m_count = 1;
			m_selected.m_type = SelectedType::NavMeshVertex;
			m_renderAspect.updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

			result = true;
		}
	}

	return result;
}

bool EditorEngine::multiSelectNavMeshVertex(s32 mouseX, s32 mouseY)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto selectedIndex = getSelectedNavMeshVertex(mouseX, mouseY);
		if (selectedIndex != 0xffffffff)
		{
			auto &navMesh = m_pEditorScene->getNavMesh();

			auto index = m_selected.m_navMeshVertices.m_count;

			m_selected.m_navMeshVertices.m_vertices[index] = selectedIndex;
			++m_selected.m_navMeshVertices.m_count;
			m_selected.m_navMeshVertices.m_count = std::min(m_selected.m_navMeshVertices.m_count, (u8)3u);

			m_selected.m_type = SelectedType::NavMeshVertex;
			m_renderAspect.updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

			result = true;
		}
	}

	return result;
}

void EditorEngine::deselectNavMeshVertex()
{
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		m_renderAspect.updateNavMeshBuffer(navMesh);
		m_selected.m_navMeshVertices.m_count = 0;
	}
	m_selected.m_item = nullptr;
}

bool EditorEngine::createNavMeshTriangleFromSelectedVertices()
{
	bool result = false;

	if (m_selected.m_navMeshVertices.m_count == 3)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.addTriangle(m_selected.m_navMeshVertices.m_vertices);

		buildNavGraph();

		m_renderAspect.updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);
	}

	return result;
}

void EditorEngine::setSelectedNavMeshVertexPosition(const vx::float3 &position)
{
	if (m_selected.m_navMeshVertices.m_count != 0)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();

		auto selectedIndex = m_selected.m_navMeshVertices.m_vertices[0];
		navMesh.setVertexPosition(selectedIndex, position);

		m_renderAspect.updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);
		buildNavGraph();
	}
}

vx::float3 EditorEngine::getSelectedNavMeshVertexPosition() const
{
	vx::float3 result;
	if (m_selected.m_navMeshVertices.m_count != 0)
	{
		auto selectedIndex = m_selected.m_navMeshVertices.m_vertices[0];

		auto &navMesh = m_pEditorScene->getNavMesh();
		auto vertices = navMesh.getVertices();

		result = vertices[selectedIndex];
	}
	return result;
}

void EditorEngine::createLight()
{
	if (m_pEditorScene)
	{
		Light light;
		light.m_position = vx::float3(0);
		light.m_falloff = 5.0f;
		light.m_lumen = 100.0f;

		m_selected.m_item = m_pEditorScene->addLight(light);
		m_selected.m_type = SelectedType::Light;

		auto lightCount = m_pEditorScene->getLightCount();
		auto lights = m_pEditorScene->getLights();

		m_renderAspect.updateLightBuffer(lights, lightCount);
	}
}

bool EditorEngine::selectLight(s32 mouseX, s32 mouseY)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto ray = getRay(mouseX, mouseY);
		auto selectedLight = m_pEditorScene->getLight(ray);

		if (selectedLight)
		{
			m_selected.m_type = SelectedType::Light;
			m_selected.m_item = selectedLight;
		}

		result = (selectedLight != nullptr);
	}

	return result;
}

void EditorEngine::deselectLight()
{
	if (m_pEditorScene)
	{
		if (m_selected.m_type == SelectedType::Light)
		{
			m_selected.m_item = nullptr;
		}
	}
}

void EditorEngine::getSelectLightPosition(vx::float3* position)
{
	if (m_pEditorScene &&
		m_selected.m_type == SelectedType::Light &&
		m_selected.m_item)
	{
		Light* ptr = (Light*)m_selected.m_item;
		*position = ptr->m_position;
	}
}

void EditorEngine::setSelectLightPosition(const vx::float3 &position)
{
	if (m_pEditorScene &&
		m_selected.m_type == SelectedType::Light &&
		m_selected.m_item)
	{
		Light* ptr = (Light*)m_selected.m_item;
		ptr->m_position = position;

		m_pEditorScene->updateLightPositions();

		auto lightCount = m_pEditorScene->getLightCount();
		auto lights = m_pEditorScene->getLights();

		m_renderAspect.updateLightBuffer(lights, lightCount);
	}
}

SelectedType EditorEngine::getSelectedItemType() const
{
	return m_selected.m_type;
}

EditorScene* EditorEngine::getEditorScene() const
{
	return m_pEditorScene;
}

void EditorEngine::showNavmesh(bool b)
{
	m_renderAspect.showNavmesh(b);
}

void EditorEngine::showInfluenceMap(bool b)
{
	m_renderAspect.showInfluenceMap(b);
}

void EditorEngine::addWaypoint(s32 mouseX, s32 mouseY)
{
	vx::float3 hitPos;
	auto ptr = raytraceAgainstStaticMeshes(mouseX, mouseY, &hitPos);
	if (ptr)
	{
		// todo
	}
}

u32 EditorEngine::getMeshCount() const
{
	if (m_pEditorScene)
		return m_pEditorScene->getMeshes().size();

	return 0;
}

const char* EditorEngine::getMeshName(u32 i) const
{
	const char* meshName = nullptr;
	if (m_pEditorScene)
	{
		auto &meshes = m_pEditorScene->getMeshes();
		auto sid = meshes.keys()[i];

		meshName = m_pEditorScene->getMeshName(sid);
	}

	return meshName;
}

u64 EditorEngine::getMeshSid(u32 i) const
{
	u64 sidValue = 0;

	if (m_pEditorScene)
	{
		auto &meshes = m_pEditorScene->getMeshes();
		auto sid = meshes.keys()[i];

		sidValue = sid.value;
	}

	return sidValue;
}

u32 EditorEngine::getMaterialCount() const
{
	u32 count = 0;

	if (m_pEditorScene)
	{
		count = m_pEditorScene->getMaterialCount();
	}

	return count;
}

const char* EditorEngine::getMaterialName(u32 i) const
{
	const char* name = nullptr;

	if (m_pEditorScene)
	{
		auto &materials = m_pEditorScene->getMaterials();
		auto sid = materials.keys()[i];

		name = m_pEditorScene->getMaterialName(sid);
	}

	return name;
}

u64 EditorEngine::getMaterialSid(u32 i) const
{
	u64 sidValue = 0;

	if (m_pEditorScene)
	{
		auto &materials = m_pEditorScene->getMaterials();
		auto sid = materials.keys()[i];

		sidValue = sid.value;
	}

	return sidValue;
}