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
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/Ray.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/NavMeshGraph.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib/debugPrint.h>
#include "developer.h"
#include <vxEngineLib/FileEvents.h>
#include <vxEngineLib/Reference.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Actor.h>
#include "EngineGlobals.h"
#include <vxLib/Graphics/Camera.h>
#include <vxEngineLib/MeshFile.h>

#include <Dbghelp.h>

u32 g_editorTypeMesh{ 0xffffffff };
u32 g_editorTypeMaterial{ 0xffffffff };
u32 g_editorTypeScene{ 0xffffffff };
u32 g_editorTypeFbx{ 0xffffffff };
u32 g_editorTypeAnimation{ 0xffffffff };

EditorEngine::EditorEngine()
	:m_eventManager(),
	m_physicsAspect(),
	m_renderAspect(),
	m_fileAspect(),
	m_bRunFileThread(),
	m_fileAspectThread(),
	m_previousSceneLoaded(false)
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
	m_memory = Memory(128 MBYTE, 64);

	m_allocator = vx::StackAllocator(m_memory.get(), m_memory.size());

	m_scratchAllocator = vx::StackAllocator(m_allocator.allocate(1 MBYTE, 64), 1 MBYTE);

	m_eventManager.initialize(&m_allocator, 256);

	if (!m_fileAspect.initialize(&m_allocator, dataDir, &m_eventManager, m_physicsAspect.getCooking()))
		return false;

	Locator::provide(&m_fileAspect);

	return true;
}

bool EditorEngine::createRenderAspectGL(const std::string &dataDir, const RenderAspectDescription &desc)
{

	//auto handle = LoadLibrary(L"../../../lib/vs2013/vxRenderAspectGL_vs12_d.dll");
	auto handle = LoadLibrary(L"../../../lib/vxRenderAspectGL_d.dll");
	if (handle == nullptr)
		return false;

	auto proc = (CreateEditorRenderAspectFunction)GetProcAddress(handle, "createEditorRenderAspect");
	auto procDestroy = (DestroyEditorRenderAspectFunction)GetProcAddress(handle, "destroyEditorRenderAspect");
	if (proc == nullptr || procDestroy == nullptr)
		return false;

	RenderAspectInitializeError error;
	auto renderAspect = proc(desc, &error);
	if (renderAspect == nullptr)
		return false;

	m_renderAspect = renderAspect;
	m_renderAspectDll = handle;
	m_destroyFn = procDestroy;

	return true;
}

bool EditorEngine::initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, Editor::Scene* pScene)
{
	vx::activateChannel(vx::debugPrint::Channel_Render);
	vx::activateChannel(vx::debugPrint::Channel_Editor);
	vx::activateChannel(vx::debugPrint::Channel_FileAspect);
	vx::DebugPrint::g_verbosity = 1;

	const std::string dataDir("../../data/");
	m_pEditorScene = pScene;
	m_resolution = resolution;
	m_panel = panel;

	if (!m_physicsAspect.initialize())
	{
		return false;
	}

	if (!initializeImpl(dataDir))
		return false;

	g_engineConfig.m_fov = 66.0f;
	g_engineConfig.m_zFar = 250.0f;
	g_engineConfig.m_renderDebug = true;
	g_engineConfig.m_resolution = resolution;
	g_engineConfig.m_vsync = false;
	g_engineConfig.m_zNear = 0.1f;
	g_engineConfig.m_editor = true;

	g_engineConfig.m_rendererSettings.m_renderMode = Graphics::RendererSettings::Mode_GL;
	g_engineConfig.m_rendererSettings.m_shadowMode = 0;
	g_engineConfig.m_rendererSettings.m_voxelGIMode = 0;
	g_engineConfig.m_rendererSettings.m_maxMeshInstances = 150;

	RenderAspectDescription renderAspectDesc =
	{
		dataDir,
		panel,
		tmp,
		&m_allocator,
		&g_engineConfig,
		&m_fileAspect,
		&m_eventManager
	};
	//renderAspectDesc.hwnd = m_panel;

	if (!createRenderAspectGL(dataDir, renderAspectDesc))
	{
		return false;
	}

	//dev::g_debugRenderSettings.setVoxelize(0);
	//dev::g_debugRenderSettings.setShadingMode(ShadingMode::Albedo);
	RenderUpdateTask task;
	task.type = RenderUpdateTask::Type::ToggleRenderMode;
	//m_renderAspect->queueUpdateTask(task);

	Locator::provide(&m_physicsAspect);

	m_eventManager.initialize(&m_allocator, 255);
	m_eventManager.registerListener(m_renderAspect, 1, (u8)vx::EventType::File_Event);
	m_eventManager.registerListener(&m_physicsAspect, 1, (u8)vx::EventType::File_Event);
	m_eventManager.registerListener(this, 1, (u8)vx::EventType::File_Event);

	//m_bRun = 1;
	m_bRunFileThread.store(1);
	m_shutdown = 0;

	memset(&m_selected, 0, sizeof(m_selected));

	return true;
}

void EditorEngine::shutdownEditor()
{
	m_fileAspectThread.join();
	m_fileAspect.shutdown();

	m_physicsAspect.shutdown();
	if (m_renderAspect)
	{
		m_renderAspect->shutdown(m_panel);
		m_destroyFn(m_renderAspect);
		m_renderAspect = nullptr;
	}
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

	m_influenceMap.initialize(navMesh, m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());

	NavMeshGraph graph;
	graph.initialize(navMesh);

	m_renderAspect->updateInfluenceCellBuffer(m_influenceMap);
	m_renderAspect->updateNavMeshGraphNodesBuffer(graph);
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
		auto sid = vx::StringID(evt.arg1.u64);
		auto pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);

		if (call_editorCallback(sid))
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded mesh %llu %s", sid.value, pStr->c_str());
			auto meshFile = m_fileAspect.getMesh(sid);

			m_pEditorScene->addMesh(sid, pStr->c_str(), meshFile);

		}
		delete(pStr);

	}break;
	case vx::FileEvent::Texture_Loaded:
		break;
	case vx::FileEvent::Material_Loaded:
	{
		auto sid = vx::StringID(evt.arg1.u64);
		auto pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);
		if (call_editorCallback(sid))
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded material %llu  %s", sid.value, pStr->c_str());
			auto material = m_fileAspect.getMaterial(sid);
			m_pEditorScene->addMaterial(sid, pStr->c_str(), material);
		}
		delete(pStr);

	}break;
	case vx::FileEvent::EditorScene_Loaded:
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded Scene");
		call_editorCallback(vx::StringID(evt.arg1.u64));

		buildNavGraph();
		m_renderAspect->updateWaypoints(m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());
	}break;
	case vx::FileEvent::Animation_Loaded:
	{
		auto sid = vx::StringID(evt.arg1.u64);
		if (evt.arg2.ptr)
		{
			auto pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);

			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded Animation %llu", sid.value);
			call_editorCallback(sid);

			m_pEditorScene->addAnimation(sid, std::move(*pStr));
		}
		else
		{
			auto str = m_fileAspect.getAnimationName(sid);
			if (str)
				m_pEditorScene->addAnimation(sid, std::move(std::string(str)));
		}
	}break;
	default:
	{
	}break;
	}
}

void EditorEngine::requestLoadFile(const vx::FileEntry &fileEntry, void* p)
{
	m_fileAspect.requestLoadFile(fileEntry, p);
}

void EditorEngine::editor_saveScene(const char* name)
{
	auto sceneCopy = new Editor::Scene();
	m_pEditorScene->copy(sceneCopy);
	m_fileAspect.requestSaveFile(vx::FileEntry(name, vx::FileType::EditorScene), sceneCopy);
}

void EditorEngine::editor_setTypes(u32 mesh, u32 material, u32 scene, u32 fbx, u32 typeAnimation)
{
	g_editorTypeMesh = mesh;
	g_editorTypeMaterial = material;
	g_editorTypeScene = scene;
	g_editorTypeFbx = fbx;
	g_editorTypeAnimation = typeAnimation;
}

void EditorEngine::editor_start()
{
	m_fileAspectThread = vx::thread(&EditorEngine::loopFileThread, this);
}

void EditorEngine::editor_render()
{
	m_eventManager.update();

	m_renderAspect->update();

	m_renderAspect->submitCommands();

	m_renderAspect->endFrame();
}

void EditorEngine::editor_loadFile(const char *filename, u32 type, Editor::LoadFileCallback f)
{
	void *p = nullptr;
	vx::FileEntry fileEntry;
	if (type == g_editorTypeMesh)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Mesh);
		p = new std::string(filename);

		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Trying to load mesh %llu '%s'", fileEntry.getSid().value, filename);
	}
	else if (type == g_editorTypeMaterial)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Material);
		p = new std::string(filename);

		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Trying to load material %llu '%s'", fileEntry.getSid().value, filename);
	}
	else if (type == g_editorTypeScene)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::EditorScene);

		if (m_previousSceneLoaded)
		{
			VX_ASSERT(false);
		}

		p = m_pEditorScene;
	}
	else if (type == g_editorTypeFbx)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Fbx);
		p = new std::string(filename);
	}
	else if (type == g_editorTypeAnimation)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Animation);
		p = new std::string(filename);
	}
	else
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Trying to load unknown file type");
		//assert(false);
	}

	vx::lock_guard<vx::mutex> guard(m_editorMutex);
	m_requestedFiles.insert(fileEntry.getSid(), std::make_pair(f, type));

	m_fileAspect.requestLoadFile(fileEntry, p);
}

void EditorEngine::editor_moveCamera(f32 dirX, f32 dirY, f32 dirZ)
{
	m_renderAspect->moveCamera(dirX, dirY, dirZ);
}

void EditorEngine::editor_rotateCamera(f32 dirX, f32 dirY, f32)
{
	static f32 x = 0.0f;
	static f32 y = 0.0f;

	x += dirX * 0.01f;
	y += dirY * 0.01f;

	vx::float4a rot(y, x, 0, 0);
	auto v = vx::quaternionRotationRollPitchYawFromVector(rot);

	m_renderAspect->rotateCamera(v);
}

bool EditorEngine::call_editorCallback(const vx::StringID &sid)
{
	bool result = false;

	vx::lock_guard<vx::mutex> guard(m_editorMutex);
	auto it = m_requestedFiles.find(sid);
	if (it != m_requestedFiles.end())
	{
		(*it->first)(sid.value, it->second);
		m_requestedFiles.erase(it);
		result = true;
	}

	return result;
}

vx::float4a EditorEngine::getRayDir(s32 mouseX, s32 mouseY)
{
	f32 ndc_x = f32(mouseX) / m_resolution.x;
	f32 ndc_y = f32(mouseY) / m_resolution.y;

	ndc_x = ndc_x * 2.0f - 1.0f;
	ndc_y = 1.0f - ndc_y * 2.0f;

	vx::mat4 projMatrix;
	m_renderAspect->getProjectionMatrix(&projMatrix);
	auto invProjMatrix = vx::MatrixInverse(projMatrix);

	vx::float4a ray_clip(ndc_x, ndc_y, -1, 1);
	vx::float4a ray_eye = vx::Vector4Transform(invProjMatrix, ray_clip);
	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	vx::mat4 viewMatrix;
	m_renderAspect->getCamera().getViewMatrix(&viewMatrix);
	auto inverseViewMatrix = vx::MatrixInverse(viewMatrix);
	vx::float4a ray_world = vx::Vector4Transform(inverseViewMatrix, ray_eye);
	ray_world = vx::normalize3(ray_world);

	return ray_world;
}

vx::StringID EditorEngine::raytraceAgainstStaticMeshes(s32 mouseX, s32 mouseY, vx::float3* hitPosition)
{
	auto ray_world = getRayDir(mouseX, mouseY);

	auto cameraPosition = m_renderAspect->getCamera().getPosition();

	vx::float4a tmp = cameraPosition;
	f32 distance;
	return m_physicsAspect.raycast_static(tmp, ray_world, 50.0f, hitPosition, &distance);
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
	auto meshInstances = m_pEditorScene->getMeshInstancesEditor();
	auto sid = meshInstances[i].getNameSid();
	return getMeshInstanceName(sid);
}

const char* EditorEngine::getMeshInstanceName(const vx::StringID &sid) const
{
	return m_pEditorScene->getMeshInstanceName(sid);
}

u64 EditorEngine::getMeshInstanceSid(u32 i) const
{
	u64 sidValue = 0;

	if (m_pEditorScene)
	{
		auto meshInstances = m_pEditorScene->getMeshInstancesEditor();
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

u64 EditorEngine::getMeshInstanceMeshSid(u64 instanceSid) const
{
	u64 sidValue = 0;

	auto meshInstance = m_pEditorScene->getMeshInstance(vx::StringID(instanceSid));
	if (meshInstance != nullptr)
	{
		sidValue = meshInstance->getMeshSid().value;
	}

	return sidValue;
}

void EditorEngine::setMeshInstanceMeshSid(u64 instanceSid, u64 meshSid)
{
	auto meshInstance = m_pEditorScene->getMeshInstance(vx::StringID(instanceSid));
	if (meshInstance != nullptr)
	{
		m_renderAspect->setMeshInstanceMesh(vx::StringID(instanceSid), vx::StringID(meshSid));
		meshInstance->setMeshSid(vx::StringID(meshSid));

		m_physicsAspect.editorSetStaticMeshInstanceMesh(meshInstance->getMeshInstance());
	}
}

u64 EditorEngine::getMeshInstanceMaterialSid(u64 instanceSid) const
{
	u64 sidValue = 0;

	auto sid = vx::StringID(instanceSid);
	auto meshInstance = m_pEditorScene->getMeshInstance(sid);
	if (meshInstance)
	{
		auto &material = meshInstance->getMaterial();
		sidValue = (*material).getSid().value;
	}

	return sidValue;
}

void EditorEngine::getMeshInstancePosition(u64 sid, vx::float3* position)
{
	auto instance = m_pEditorScene->getMeshInstance(vx::StringID(sid));

	auto &transform = instance->getTransform();
	*position = transform.m_translation;
}

bool EditorEngine::selectMeshInstance(s32 x, s32 y)
{
	bool result = false;
	if (m_pEditorScene)
	{
		vx::float3 p;
		auto sid = raytraceAgainstStaticMeshes(x, y, &p);
		if (sid.value != 0)
		{
			auto debugName = getMeshInstanceName(sid);
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "selected instance %s", debugName);

			auto ptr = m_pEditorScene->getMeshInstance(sid);
			m_renderAspect->setSelectedMeshInstance(ptr);
			m_selected.m_type = SelectedType::MeshInstance;
			m_selected.m_item = (void*)ptr;

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
		auto instances = m_pEditorScene->getMeshInstancesEditor();

		m_renderAspect->setSelectedMeshInstance(&instances[i]);
		m_selected.m_type = SelectedType::MeshInstance;
		m_selected.m_item = (void*)&instances[i];

		result = true;
	}

	return result;
}

bool EditorEngine::selectMeshInstance(u64 sid)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto instance = m_pEditorScene->getMeshInstance(vx::StringID(sid));
		VX_ASSERT(instance != nullptr);

		m_selected.m_type = SelectedType::MeshInstance;
		m_selected.m_item = (void*)instance;

		result = true;
	}

	return result;
}

u64 EditorEngine::deselectMeshInstance()
{
	u64 result = 0;
	if (m_pEditorScene)
	{
		auto selectedInstance = (Editor::MeshInstance*)m_selected.m_item;
		if (selectedInstance)
		{
			result = selectedInstance->getNameSid().value;

			m_renderAspect->setSelectedMeshInstance(nullptr);
			m_selected.m_item = nullptr;
		}
	}

	return result;
}

vx::StringID EditorEngine::createMeshInstance()
{
	auto instanceSid = m_pEditorScene->createMeshInstance();
	auto instance = m_pEditorScene->getMeshInstance(instanceSid);
	m_renderAspect->addMeshInstance(*instance);

	m_physicsAspect.addMeshInstance(instance->getMeshInstance());

	return instanceSid;
}

void EditorEngine::removeMeshInstance(u64 sid)
{
	if (m_pEditorScene)
	{
		m_pEditorScene->removeMeshInstance(vx::StringID(sid));
		m_renderAspect->removeMeshInstance(vx::StringID(sid));
	}
}

void EditorEngine::setMeshInstanceMaterial(u64 instanceSid, u64 materialSid)
{
	if (m_pEditorScene)
	{
		auto meshInstance = m_pEditorScene->getMeshInstance(vx::StringID(instanceSid));

		auto sceneMaterial = m_pEditorScene->getMaterial(vx::StringID(materialSid));
		if (sceneMaterial != nullptr)
		{
			if (m_renderAspect->setSelectedMeshInstanceMaterial(*sceneMaterial))
			{
				meshInstance->setMaterial(*sceneMaterial);
			}
		}
	}
}

void EditorEngine::setMeshInstancePosition(u64 sid, const vx::float3 &p)
{
	if (m_pEditorScene)
	{
		auto instanceSid = vx::StringID(sid);
		auto instance = m_pEditorScene->getMeshInstance(instanceSid);

		instance->setTranslation(p);

		auto transform = instance->getTransform();

		m_renderAspect->setSelectedMeshInstanceTransform(transform);
		m_physicsAspect.editorSetStaticMeshInstanceTransform(instance->getMeshInstance(), instanceSid);
	}
}

void EditorEngine::setMeshInstanceRotation(u64 sid, const vx::float3 &rotationDeg)
{
	if (m_pEditorScene)
	{
		auto instanceSid = vx::StringID(sid);
		auto instance = m_pEditorScene->getMeshInstance(instanceSid);

		auto rotation = vx::degToRad(rotationDeg);
		auto r = vx::loadFloat3(rotation);

		auto q = vx::quaternionRotationRollPitchYawFromVector(r);

		vx::float4 tmp;
		vx::storeFloat4(&tmp, q);
		instance->setRotation(tmp);

		auto transform = instance->getTransform();

		m_renderAspect->setSelectedMeshInstanceTransform(transform);
		m_physicsAspect.editorSetStaticMeshInstanceTransform(instance->getMeshInstance(), instanceSid);
	}
}

void EditorEngine::getMeshInstanceRotation(u64 sid, vx::float3* rotationDeg) const
{
	if (m_pEditorScene)
	{
		auto instanceSid = vx::StringID(sid);
		auto instance = m_pEditorScene->getMeshInstance(instanceSid);

		auto transform = instance->getTransform();
		auto q = transform.m_qRotation;

		__m128 axis;
		f32 angle;
		vx::quaternionToAxisAngle(vx::loadFloat4(q), &axis, &angle);
		axis = vx::normalize3(axis);

		vx::float4a tmpAxis = axis;
		vx::angleAxisToEuler(tmpAxis, angle, rotationDeg);

	}
}

bool EditorEngine::setMeshInstanceName(u64 sid, const char* name)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto meshInstance = (MeshInstance*)m_selected.m_item;
		result = m_pEditorScene->renameMeshInstance(vx::StringID(sid), name);
	}
	return result;
}

bool EditorEngine::addNavMeshVertex(s32 mouseX, s32 mouseY, vx::float3* position)
{
	vx::float3 hitPos;
	auto sid = raytraceAgainstStaticMeshes(mouseX, mouseY, &hitPos);
	if (sid.value != 0)
	{
		auto ptr = m_pEditorScene->getMeshInstance(sid);
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.addVertex(hitPos);
		m_renderAspect->updateNavMeshBuffer(navMesh);

		*position = hitPos;
	}

	return sid.value != 0;
}

void EditorEngine::removeNavMeshVertex(const vx::float3 &position)
{
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.removeVertex(position);

		buildNavGraph();
		m_renderAspect->updateNavMeshBuffer(navMesh);
	}
}

void EditorEngine::removeSelectedNavMeshVertex()
{
	if (m_selected.m_navMeshVertices.m_count != 0)
	{
		auto index = m_selected.m_navMeshVertices.m_vertices[0];
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.removeVertex(index);

		m_renderAspect->updateNavMeshBuffer(navMesh);
	}
}

Ray EditorEngine::getRay(s32 mouseX, s32 mouseY)
{
	auto rayDir = getRayDir(mouseX, mouseY);
	auto cameraPosition = m_renderAspect->getCamera().getPosition();

	Ray ray;
	vx::storeFloat3(&ray.o, cameraPosition);
	vx::storeFloat3(&ray.d, rayDir);
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
			m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

			result = true;
		}
	}

	return result;
}

bool EditorEngine::selectNavMeshVertexIndex(u32 index)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();

		m_selected.m_navMeshVertices.m_vertices[0] = index;
		m_selected.m_navMeshVertices.m_count = 1;
		m_selected.m_type = SelectedType::NavMeshVertex;
		m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

		result = true;
	}

	return result;
}

bool EditorEngine::selectNavMeshVertexPosition(const vx::float3 &position)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();

		u32 index = 0;
		if (navMesh.getIndex(position, &index))
		{
			m_selected.m_navMeshVertices.m_vertices[0] = index;
			m_selected.m_navMeshVertices.m_count = 1;
			m_selected.m_type = SelectedType::NavMeshVertex;
			m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

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
			m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

			result = true;
		}
	}

	return result;
}

u32 EditorEngine::deselectNavMeshVertex()
{
	u32 index = 0;
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		m_renderAspect->updateNavMeshBuffer(navMesh);

		index = m_selected.m_navMeshVertices.m_vertices[0];
		m_selected.m_navMeshVertices.m_count = 0;
	}
	m_selected.m_item = nullptr;

	return index;
}

bool EditorEngine::createNavMeshTriangleFromSelectedVertices(vx::uint3* selected)
{
	bool result = false;

	if (m_selected.m_navMeshVertices.m_count == 3)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.addTriangle(m_selected.m_navMeshVertices.m_vertices);

		buildNavGraph();

		m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);

		selected->x = m_selected.m_navMeshVertices.m_vertices[0];
		selected->y = m_selected.m_navMeshVertices.m_vertices[1];
		selected->z = m_selected.m_navMeshVertices.m_vertices[2];

		result = true;
	}

	return result;
}

void EditorEngine::createNavMeshTriangleFromIndices(const vx::uint3 &indices)
{
	if (m_pEditorScene)
	{
		m_selected.m_navMeshVertices.m_count = 3;
		m_selected.m_navMeshVertices.m_vertices[0] = indices.x;
		m_selected.m_navMeshVertices.m_vertices[1] = indices.y;
		m_selected.m_navMeshVertices.m_vertices[2] = indices.z;

		vx::uint3 tmp;
		createNavMeshTriangleFromSelectedVertices(&tmp);
	}
}

void EditorEngine::removeNavMeshTriangle()
{
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		navMesh.removeTriangle();

		m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);
		buildNavGraph();
	}
}

u32 EditorEngine::getSelectedNavMeshCount() const
{
	return m_selected.m_navMeshVertices.m_count;
}

void EditorEngine::setSelectedNavMeshVertexPosition(const vx::float3 &position)
{
	if (m_selected.m_navMeshVertices.m_count != 0)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();

		auto selectedIndex = m_selected.m_navMeshVertices.m_vertices[0];
		navMesh.setVertexPosition(selectedIndex, position);

		m_renderAspect->updateNavMeshBuffer(navMesh, m_selected.m_navMeshVertices.m_vertices, m_selected.m_navMeshVertices.m_count);
		buildNavGraph();
	}
}

bool EditorEngine::getSelectedNavMeshVertexPosition(vx::float3* p) const
{
	bool result = false;
	if (m_selected.m_navMeshVertices.m_count != 0)
	{
		auto selectedIndex = m_selected.m_navMeshVertices.m_vertices[0];

		auto &navMesh = m_pEditorScene->getNavMesh();
		auto vertices = navMesh.getVertices();

		*p = vertices[selectedIndex];
		result = true;
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

		m_renderAspect->updateLightBuffer(lights, lightCount);
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

void EditorEngine::getSelectLightPosition(vx::float3* position) const
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

		m_renderAspect->updateLightBuffer(lights, lightCount);
	}
}

float EditorEngine::getSelectLightFalloff() const
{
	f32 falloff = 0.0f;
	if (m_pEditorScene &&
		m_selected.m_type == SelectedType::Light &&
		m_selected.m_item)
	{
		Light* ptr = (Light*)m_selected.m_item;
		falloff = ptr->m_falloff;
	}
	return falloff;
}

f32 EditorEngine::getSelectLightLumen() const
{
	f32 value = 0.0f;

	if (m_pEditorScene &&
		m_selected.m_type == SelectedType::Light &&
		m_selected.m_item)
	{
		Light* ptr = (Light*)m_selected.m_item;
		value = ptr->m_lumen;
	}

	return value;
}

void EditorEngine::setSelectLightLumen(f32 lumen)
{
	if (m_pEditorScene &&
		m_selected.m_type == SelectedType::Light &&
		m_selected.m_item)
	{
		Light* ptr = (Light*)m_selected.m_item;
		ptr->m_lumen = lumen;

		m_pEditorScene->updateLightPositions();

		auto lightCount = m_pEditorScene->getLightCount();
		auto lights = m_pEditorScene->getLights();

		m_renderAspect->updateLightBuffer(lights, lightCount);
	}
}

void EditorEngine::setSelectLightFalloff(f32 falloff)
{
	if (m_pEditorScene &&
		m_selected.m_type == SelectedType::Light &&
		m_selected.m_item)
	{
		Light* ptr = (Light*)m_selected.m_item;
		ptr->m_falloff = falloff;

		m_pEditorScene->updateLightPositions();

		auto lightCount = m_pEditorScene->getLightCount();
		auto lights = m_pEditorScene->getLights();

		m_renderAspect->updateLightBuffer(lights, lightCount);
	}
}

SelectedType EditorEngine::getSelectedItemType() const
{
	return m_selected.m_type;
}

Editor::Scene* EditorEngine::getEditorScene() const
{
	return m_pEditorScene;
}

void EditorEngine::showNavmesh(bool b)
{
	//m_renderAspect->showNavmesh(b);
}

void EditorEngine::showInfluenceMap(bool b)
{
	//m_renderAspect->showInfluenceMap(b);
}

bool EditorEngine::addWaypoint(s32 mouseX, s32 mouseY, vx::float3* position)
{
	bool result = false;

	vx::float3 hitPos;
	auto sid = raytraceAgainstStaticMeshes(mouseX, mouseY, &hitPos);
	if (sid.value != 0)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		if (navMesh.contains(hitPos))
		{
			m_pEditorScene->addWaypoint(hitPos);
			m_renderAspect->updateWaypoints(m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());

			*position = hitPos;
			result = true;
		}
	}

	return result;
}

void EditorEngine::addWaypoint(const vx::float3 &position)
{
	m_pEditorScene->addWaypoint(position);
	m_renderAspect->updateWaypoints(m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());
}

void EditorEngine::removeWaypoint(const vx::float3 &position)
{
	m_pEditorScene->removeWaypoint(position);
	m_renderAspect->updateWaypoints(m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());
}

void EditorEngine::addSpawn()
{
	if (m_pEditorScene)
	{
		Spawn spawn;
		spawn.type = PlayerType::AI;
		spawn.position = {0, 0, 0};

		m_pEditorScene->addSpawn(std::move(spawn));

		auto count = m_pEditorScene->getSpawnCount();
		auto spawns = m_pEditorScene->getSpawns();
		m_renderAspect->updateSpawns(spawns, count);
	}
}

bool EditorEngine::selectSpawn(s32 mouseX, s32 mouseY, u32* id)
{
	bool result = false;

	if (m_pEditorScene)
	{
		auto ray = getRay(mouseX, mouseY);
		auto spawnId = m_pEditorScene->getSpawnId(ray);
		if (spawnId != 0xffffffff)
		{
			result = true;
			*id = spawnId;
		}
	}

	return result;
}

void EditorEngine::getSpawnPosition(u32 id, vx::float3* position) const
{
	auto spawn = m_pEditorScene->getSpawn(id);
	if (spawn)
	{
		*position = spawn->position;
	}
}

u32 EditorEngine::getSpawnType(u32 id) const
{
	u32 result = 0xffffffff;

	auto spawn = m_pEditorScene->getSpawn(id);
	if (spawn)
	{
		result = (u32)spawn->type;
	}

	return result;
}

void EditorEngine::setSpawnPosition(u32 id, const vx::float3 &position)
{
	auto spawns = m_pEditorScene->getSpawns();
	auto count = m_pEditorScene->getSpawnCount();
	m_pEditorScene->setSpawnPosition(id, position);
	m_renderAspect->updateSpawns(spawns, count);
}

void EditorEngine::setSpawnType(u32 id, u32 type)
{
	m_pEditorScene->setSpawnType(id, type);
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

const char* EditorEngine::getMaterialNameIndex(u32 i) const
{
	const char* name = nullptr;

	if (m_pEditorScene)
	{
		auto materials = m_pEditorScene->getMaterials();
		auto sid = (*materials[i]).getSid();

		name = m_pEditorScene->getMaterialName(sid);
	}

	return name;
}

const char* EditorEngine::getMaterialName(u64 sid) const
{
	const char* name = nullptr;

	if (m_pEditorScene)
	{
		name = m_pEditorScene->getMaterialName(vx::StringID(sid));
	}

	return name;
}

u64 EditorEngine::getMaterialSid(u32 i) const
{
	u64 sidValue = 0;

	if (m_pEditorScene)
	{
		auto materials = m_pEditorScene->getMaterials();
		auto sid = (*materials[i]).getSid();

		sidValue = sid.value;
	}

	return sidValue;
}

void EditorEngine::setMeshInstanceAnimation(u64 instanceSid, u64 animSid)
{
	auto instance = m_pEditorScene->getMeshInstance(vx::StringID(instanceSid));
	if (instance)
	{
		instance->setAnimationSid(vx::StringID(animSid));
	}
}

u64 EditorEngine::getMeshInstanceAnimation(u64 instanceSid)
{
	u64 result = 0;
	auto instance = m_pEditorScene->getMeshInstance(vx::StringID(instanceSid));
	if (instance)
	{
		result = instance->getAnimationSid().value;
	}
	return result;
}

u32 EditorEngine::getAnimationCount() const
{
	return m_pEditorScene->getAnimationCount();
}

const char* EditorEngine::getAnimationNameIndex(u32 i) const
{
	return m_pEditorScene->getAnimationNameIndex(i);
}

u64 EditorEngine::getAnimationSidIndex(u32 i) const
{
	return m_pEditorScene->getAnimationSidIndex(i);
}

u32 EditorEngine::getMeshPhysxType(u64 sid) const
{
	u32 type = 0xffffffff;

	auto &meshes = m_pEditorScene->getMeshes();
	auto it = meshes.find(vx::StringID(sid));
	if (it != meshes.end())
	{
		type = (u32)(*it)->getPhysxMeshType();
	}

	return type;
}

void EditorEngine::setMeshPhysxType(u64 sid, u32 type)
{
	auto meshFile = m_fileAspect.getMesh(vx::StringID(sid));
	if (meshFile.isValid())
	{
		auto &meshDataAllocator = m_fileAspect.getMeshDataAllocator();

		if (m_physicsAspect.setMeshPhysxType(meshFile, (PhsyxMeshType)type, &meshDataAllocator))
		{
			auto fileName = m_fileAspect.getLoadedFileName(vx::StringID(sid));

			m_fileAspect.requestSaveFile(vx::FileEntry(fileName, vx::FileType::Mesh), meshFile.get());
		}
	}
}

u32 EditorEngine::getMeshInstanceRigidBodyType(u64 sid) const
{
	u32 type = 0xffffffff;

	auto it = m_pEditorScene->getMeshInstance(vx::StringID(sid));
	if (it != nullptr)
	{
		type = (u32)it->getRigidBodyType();
	}

	return type;
}

void EditorEngine::setMeshInstanceRigidBodyType(u64 sid, u32 type)
{
	auto instanceSid = vx::StringID(sid);
	auto editorInstance = m_pEditorScene->getMeshInstance(instanceSid);
	if (editorInstance != nullptr)
	{
		auto rigidBodyType = (PhysxRigidBodyType)type;

		if (m_physicsAspect.setMeshInstanceRigidBodyType(instanceSid, editorInstance->getMeshInstance(), rigidBodyType))
		{
			editorInstance->setRigidBodyType(rigidBodyType);
		}
	}
}