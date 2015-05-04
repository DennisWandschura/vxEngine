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
#include "Event.h"
#include "EventTypes.h"
#include "developer.h"
#include <vxLib/util/DebugPrint.h>
#include "Locator.h"
#include "MeshInstance.h"
#include "Ray.h"
#include "EditorScene.h"
#include "NavMeshGraph.h"

U32 EditorEngine::s_editorTypeMesh{ -1 };
U32 EditorEngine::s_editorTypeMaterial{ -1 };
U32 EditorEngine::s_editorTypeScene{ -1 };

EditorEngine::EditorEngine()
	:m_eventManager(),
	m_physicsAspect(m_fileAspect),
	m_renderAspect(),
	m_fileAspect(m_eventManager),
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

	const F32 fov = 66.0f;
	const auto z_near = 0.1f;
	const auto z_far = 1000.0f;
	const auto vsync = false;
	const auto debug = true;
	if (!m_renderAspect.initialize(dataDir, panel, tmp, resolution, fov, z_near, z_far, vsync, debug, &m_allocator))
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

	m_eventManager.registerListener(&m_renderAspect, 1);
	m_eventManager.registerListener(&m_physicsAspect, 1);
	m_eventManager.registerListener(this, 1);

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

	m_influenceMap.initialize(navMesh, 5, 3.0f);

	NavMeshGraph graph;
	graph.initialize(navMesh);

	m_renderAspect.updateInfluenceCellBuffer(m_influenceMap);
	m_renderAspect.updateNavMeshGraphNodesBuffer(graph);
}

void EditorEngine::handleEvent(const Event &evt)
{
	switch (evt.type)
	{
	case(EventType::File_Event) :
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void EditorEngine::handleFileEvent(const Event &evt)
{
	FileEvent fe = (FileEvent)evt.code;

	switch (fe)
	{
	case::FileEvent::Mesh_Loaded:
	{
		//vx::verboseChannelPrintF(1, dev::Channel_Editor, "Loaded Mesh");
		/*std::string* pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);
		auto sid = evt.arg1.sid;

		auto pMesh = m_fileAspect.getMesh(sid);
		//m_renderAspect.editor_addMesh(sid, pStr->c_str(), pMesh);

		call_editorCallback(sid);

		delete(pStr);*/
	}break;
	case::FileEvent::Texture_Loaded:
		break;
	case::FileEvent::Material_Loaded:
	{
		//vx::verboseChannelPrintF(1, dev::Channel_Editor, "Loaded Material");
		//std::string* pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);

		//auto pMaterial = m_fileAspect.getMaterial(sid);
		//VX_ASSERT(pMaterial != nullptr);
		//m_renderAspect.editor_addMaterial(sid, pStr->c_str(), pMaterial);

		call_editorCallback(evt.arg1.u64);

		//delete(pStr);
	}break;
	case FileEvent::Scene_Loaded:
		vx::verboseChannelPrintF(0, dev::Channel_Editor, "Loaded Scene");
		call_editorCallback(evt.arg2.u64);

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

void EditorEngine::editor_setTypes(U32 mesh, U32 material, U32 scene)
{
	s_editorTypeMesh = mesh;
	s_editorTypeMaterial = material;
	s_editorTypeScene = scene;
}

void EditorEngine::editor_start()
{
	m_fileAspectThread = vx::thread(&EditorEngine::loopFileThread, this);
}

void EditorEngine::editor_render(F32 dt)
{
	m_eventManager.update();

	m_renderAspect.update();

	m_renderAspect.render();
}

void EditorEngine::editor_loadFile(const char *filename, U32 type, Editor::LoadFileCallback f)
{
	assert(s_editorTypeMesh != s_editorTypeMaterial);

	void *p = nullptr;
	FileEntry fileEntry;
	if (type == s_editorTypeMesh)
	{
		fileEntry = FileEntry(filename, FileType::Mesh);
		p = new std::string(filename);
	}
	else if (type == s_editorTypeMaterial)
	{
		fileEntry = FileEntry(filename, FileType::Material);
		p = new std::string(filename);
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

void EditorEngine::editor_moveCamera(F32 dirX, F32 dirY, F32 dirZ)
{
	m_renderAspect.editor_moveCamera(dirX, dirY, dirZ);
}

void EditorEngine::editor_rotateCamera(F32 dirX, F32 dirY, F32 dirZ)
{
	static F32 x = 0.0f;
	static F32 y = 0.0f;

	x += dirX * 0.01f;
	y += dirY * 0.01f;

	vx::float4a rot(y, x, 0, 0);
	auto v = vx::QuaternionRotationRollPitchYawFromVector(rot);

	m_renderAspect.editor_rotateCamera(v);
}

void EditorEngine::call_editorCallback(vx::StringID sid)
{
	std::lock_guard<std::mutex> guard(m_editorMutex);
	auto it = m_requestedFiles.find(sid);
	if (it != m_requestedFiles.end())
	{
		(*it->first)(sid.value, it->second);
		m_requestedFiles.erase(it);
	}
}

vx::float4a EditorEngine::getRayDir(I32 mouseX, I32 mouseY)
{
	F32 ndc_x = F32(mouseX) / m_resolution.x;
	F32 ndc_y = F32(mouseY) / m_resolution.y;

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
	m_renderAspect.getCamera().getViewMatrix(viewMatrix);
	auto inverseViewMatrix = vx::MatrixInverse(viewMatrix);
	vx::float4a ray_world = vx::Vector4Transform(inverseViewMatrix, ray_eye);
	ray_world = vx::Vector3Normalize(ray_world);

	return ray_world;
}

MeshInstance* EditorEngine::raytraceAgainstStaticMeshes(I32 mouseX, I32 mouseY, vx::float3* hitPosition)
{
	auto ray_world = getRayDir(mouseX, mouseY);

	auto cameraPosition = m_renderAspect.getCamera().getPosition();

	return m_physicsAspect.raycast_static(vx::float3(cameraPosition.f[0], cameraPosition.f[1], cameraPosition.f[2]), vx::float3(ray_world.x, ray_world.y, ray_world.z), 50.0f, hitPosition);
}

bool EditorEngine::selectMesh(I32 x, I32 y)
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

void EditorEngine::deselectMesh()
{
	if (m_pEditorScene)
	{
		m_renderAspect.setSelectedMeshInstance(nullptr);
		m_selected.m_item = nullptr;
	}
}

void EditorEngine::updateSelectedMeshInstanceTransform(const vx::float3 &p)
{
	vx::Transform transform;
	transform.m_translation = p;
	transform.m_rotation = vx::float3(0.0f);
	transform.m_scaling = 1.0f;

	m_renderAspect.updateSelectedMeshInstanceTransform(transform);
}

void EditorEngine::addWaypoint(const vx::float3 &p)
{
	m_waypointManager.addWaypoint(p, &m_renderAspect);
}

bool EditorEngine::addNavMeshVertex(I32 mouseX, I32 mouseY)
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

Ray EditorEngine::getRay(I32 mouseX, I32 mouseY)
{
	auto rayDir = getRayDir(mouseX, mouseY);
	auto cameraPosition = m_renderAspect.getCamera().getPosition();

	Ray ray;
	vx::storeFloat(&ray.o, cameraPosition);
	vx::storeFloat(&ray.d, rayDir);
	ray.maxt = 50.0f;

	return ray;
}

U32 EditorEngine::getSelectedNavMeshVertex(I32 mouseX, I32 mouseY)
{
	auto ray = getRay(mouseX, mouseY);

	auto &navMesh = m_pEditorScene->getNavMesh();

	return navMesh.testRayAgainstVertices(ray);
}

bool EditorEngine::selectNavMeshVertex(I32 mouseX, I32 mouseY)
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

bool EditorEngine::multiSelectNavMeshVertex(I32 mouseX, I32 mouseY)
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
			m_selected.m_navMeshVertices.m_count = std::min(m_selected.m_navMeshVertices.m_count, (U8)3u);

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

bool EditorEngine::selectLight(I32 mouseX, I32 mouseY)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto ray = getRay(mouseX, mouseY);
		auto selectedLight = m_pEditorScene->getLight(ray);

		result = (selectedLight != nullptr);
	}

	return result;
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