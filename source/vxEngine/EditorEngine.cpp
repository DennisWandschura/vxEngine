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
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/Ray.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/Graphics/Light.h>
#include <vxEngineLib/NavMeshGraph.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib/debugPrint.h>
#include "developer.h"
#include <vxEngineLib/FileMessage.h>
#include <vxEngineLib/Reference.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Actor.h>
#include "EngineGlobals.h"
#include <vxLib/Graphics/Camera.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/FileEntry.h>
#include <vxEngineLib/ActorFile.h>
#include <vxEngineLib/Graphics/LightGeometryProxy.h>
#include <Dbghelp.h>

u32 g_editorTypeMesh{ 0xffffffff };
u32 g_editorTypeMaterial{ 0xffffffff };
u32 g_editorTypeScene{ 0xffffffff };
u32 g_editorTypeFbx{ 0xffffffff };
u32 g_editorTypeAnimation{ 0xffffffff };
u32 g_editorTypeActor{ 0xffffffff };

namespace EditorEngineCpp
{
	void schedulerThread(vx::TaskManager* scheduler)
	{
		std::atomic_uint running;
		running.store(1);

		scheduler->initializeThread(&running);

		while (running.load() != 0)
		{
			scheduler->swapBuffer();

			scheduler->update();
		}
	}
}

EditorEngine::EditorEngine()
	:m_msgManager(),
	m_physicsAspect(),
	m_renderAspect(),
	m_resourceAspect(),
	m_navmeshGraph(),
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

bool EditorEngine::initializeImpl(const std::string &dataDir, bool flipTextures)
{
	m_memory = Memory(g_totalMemory, 64);

	m_allocator = vx::StackAllocator(m_memory.get(), m_memory.size());

	m_scratchAllocator = vx::StackAllocator(m_allocator.allocate(1 MBYTE, 64), 1 MBYTE);

	m_msgManager.initialize(&m_allocator, 256);

	//if (!m_resourceAspect.initialize(&m_allocator, dataDir, &m_msgManager, m_physicsAspect.getCooking()))
	if (!m_resourceAspect.initialize(&m_allocator, dataDir, m_physicsAspect.getCooking(), &m_taskManager, &m_msgManager, flipTextures, true))
		return false;

	Locator::provide(&m_resourceAspect);

	return true;
}

bool EditorEngine::createRenderAspectGL(const std::string &dataDir, const RenderAspectDescription &desc, AbortSignalHandlerFun signalHandlerFn)
{
	auto handle = LoadLibrary(L"../../../lib/vxRenderAspectGL_d.dll");
	if (handle == nullptr)
		return false;

	auto proc = (CreateEditorRenderAspectFunction)GetProcAddress(handle, "createEditorRenderAspect");
	auto procDestroy = (DestroyEditorRenderAspectFunction)GetProcAddress(handle, "destroyEditorRenderAspect");
	if (proc == nullptr || procDestroy == nullptr)
		return false;

	auto renderAspect = proc();
	if (renderAspect == nullptr)
		return false;

	m_renderAspect = renderAspect;
	m_renderAspectDll = handle;
	m_destroyFn = procDestroy;

	return true;
}

bool EditorEngine::initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, AbortSignalHandlerFun signalHandlerFn, Editor::Scene* pScene, Logfile* logfile)
{
	vx::activateChannel(vx::debugPrint::Channel_Render);
	vx::activateChannel(vx::debugPrint::Channel_Editor);
	vx::activateChannel(vx::debugPrint::Channel_FileAspect);
	vx::debugPrint::g_verbosity = 1;

	const std::string dataDir("../../data/");
	m_pEditorScene = pScene;
	m_resolution = resolution;
	m_panel = panel;

	if (!m_physicsAspect.initialize(&m_taskManager))
	{
		return false;
	}

	g_engineConfig.m_fovDeg = 66.0f;
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

	g_engineConfig.m_rendererSettings.m_voxelSettings.m_voxelGridDim = 16;
	g_engineConfig.m_rendererSettings.m_voxelSettings.m_voxelTextureSize = 128;

	bool flipTextures = (g_engineConfig.m_rendererSettings.m_renderMode == Graphics::RendererSettings::Mode_GL);
	if (!initializeImpl(dataDir, flipTextures))
		return false;

	m_taskManager.initialize(2, 10, 30.0f, &m_allocator);

	m_taskManagerThread = std::thread(EditorEngineCpp::schedulerThread, &m_taskManager);

	RenderAspectDescription renderAspectDesc =
	{
		dataDir,
		panel,
		tmp,
		&m_allocator,
		&g_engineConfig,
		logfile,
		nullptr,
		&m_resourceAspect,
		&m_msgManager,
		&m_taskManager
	};
	//renderAspectDesc.hwnd = m_panel;

	if (!createRenderAspectGL(dataDir, renderAspectDesc, signalHandlerFn))
	{
		return false;
	}

	if (m_renderAspect->initialize(renderAspectDesc, signalHandlerFn) != RenderAspectInitializeError::OK)
	{
		return false;
	}

	//dev::g_debugRenderSettings.setVoxelize(0);
	//dev::g_debugRenderSettings.setShadingMode(ShadingMode::Albedo);
	//m_renderAspect->queueUpdateTask(task);

	Locator::provide(&m_physicsAspect);

	m_msgManager.initialize(&m_allocator, 255);
	m_msgManager.registerListener(m_renderAspect, 1, (u8)vx::MessageType::File | (u8)vx::MessageType::Renderer);
	m_msgManager.registerListener(&m_physicsAspect, 1, (u8)vx::MessageType::File);
	m_msgManager.registerListener(this, 1, (u8)vx::MessageType::File);

	//m_bRun = 1;
	m_shutdown = 0;

	memset(&m_selected, 0, sizeof(m_selected));

	return true;
}

void EditorEngine::shutdownEditor()
{
	if (m_taskManagerThread.joinable())
		m_taskManagerThread.join();

	m_taskManager.shutdown();

	m_resourceAspect.shutdown();

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
	m_taskManager.stop();
}

void EditorEngine::buildNavGraph()
{
	auto &navMesh = m_pEditorScene->getNavMesh();

	m_influenceMap.initialize(navMesh, m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());

	m_navmeshGraph.initialize(navMesh);

	m_renderAspect->updateInfluenceCellBuffer(m_influenceMap);
	m_renderAspect->updateNavMeshGraphNodesBuffer(m_navmeshGraph);
}

void EditorEngine::handleMessage(const vx::Message &evt)
{
	switch (evt.type)
	{
	case(vx::MessageType::File) :
		handleFileEvent(evt);
		break;
	default:
		break;
	}
}

void EditorEngine::handleFileEvent(const vx::Message &evt)
{
	auto fe = (vx::FileMessage)evt.code;

	switch (fe)
	{
	case vx::FileMessage::Mesh_Loaded:
	{
		auto sid = vx::StringID(evt.arg1.u64);
		auto pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);

		if (call_editorCallback(sid))
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded mesh %llu %s", sid.value, pStr->c_str());
			auto meshFile = m_resourceAspect.getMesh(sid);

			m_pEditorScene->addMesh(sid, pStr->c_str(), meshFile);

		}
		delete(pStr);

	}break;
	case vx::FileMessage::Texture_Loaded:
		break;
	case vx::FileMessage::Material_Loaded:
	{
		auto sid = vx::StringID(evt.arg1.u64);
		auto pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);
		if (call_editorCallback(sid))
		{
			vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded material %llu  %s", sid.value, pStr->c_str());
			Material* material = m_resourceAspect.getMaterial(sid);
			m_pEditorScene->addMaterial(sid, pStr->c_str(), material);
		}
		delete(pStr);

	}break;
	case vx::FileMessage::EditorScene_Loaded:
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Loaded Scene");
		call_editorCallback(vx::StringID(evt.arg1.u64));

		buildNavGraph();
		auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();

		m_renderAspect->updateWaypoints(m_pEditorScene->getWaypoints(), m_pEditorScene->getWaypointCount());
		m_renderAspect->updateJoints(m_pEditorScene->getJoints(), m_pEditorScene->getJointCount(), sortedInstances);
		m_renderAspect->updateSpawns(m_pEditorScene->getSpawns(), m_pEditorScene->getSpawnCount());
		m_renderAspect->updateLightGeometryProxies(m_pEditorScene->getLightGeometryProxies(), m_pEditorScene->getLightGeometryProxyCount());
	}break;
	case vx::FileMessage::Animation_Loaded:
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
			VX_ASSERT(false);
			//auto str = m_resourceAspect.getAnimationName(sid);
			//if (str)
			//	m_pEditorScene->addAnimation(sid, std::move(std::string(str)));
		}
	}break;
	case vx::FileMessage::Actor_Loaded:
	{
		auto sid = vx::StringID(evt.arg1.u64);
		auto pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);

		call_editorCallback(sid);
		delete(pStr);

	}break;
	default:
	{
	}break;
	}
}

void EditorEngine::requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg)
{
	m_resourceAspect.requestLoadFile(fileEntry, arg);
}

void EditorEngine::editor_saveScene(const char* name)
{
	auto sceneCopy = new Editor::Scene();
	m_pEditorScene->copy(sceneCopy);
	vx::Variant arg;
	arg.ptr = sceneCopy;

	m_resourceAspect.requestSaveFile(vx::FileEntry(name, vx::FileType::EditorScene), arg);
}

void EditorEngine::editor_setTypes(u32 mesh, u32 material, u32 scene, u32 fbx, u32 typeAnimation, u32 typeActor)
{
	g_editorTypeMesh = mesh;
	g_editorTypeMaterial = material;
	g_editorTypeScene = scene;
	g_editorTypeFbx = fbx;
	g_editorTypeAnimation = typeAnimation;
	g_editorTypeActor = typeActor;
}

void EditorEngine::update(f32 dt)
{
	m_msgManager.update();

	//m_physicsAspect.update(1.0f/30.f);
	//m_physicsAspect.fetch();

	m_resourceAspect.update();

	m_renderAspect->update();

	m_renderAspect->buildCommands();
	m_renderAspect->submitCommands();
	m_renderAspect->wait();
	m_renderAspect->swapBuffers();
}

void EditorEngine::editor_loadFile(const char *filename, u32 type, Editor::LoadFileCallback f, vx::Variant userArg)
{
	vx::Variant arg;
	arg.ptr = nullptr;
	vx::FileEntry fileEntry;
	if (type == g_editorTypeMesh)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Mesh);
		arg.ptr = new std::string(filename);

		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Trying to load mesh %llu '%s'", fileEntry.getSid().value, filename);
	}
	else if (type == g_editorTypeMaterial)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Material);
		arg.ptr = new std::string(filename);

		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Trying to load material %llu '%s'", fileEntry.getSid().value, filename);
	}
	else if (type == g_editorTypeScene)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::EditorScene);

		if (m_previousSceneLoaded)
		{
			VX_ASSERT(false);
		}

		arg.ptr = m_pEditorScene;
	}
	else if (type == g_editorTypeFbx)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Fbx);
		arg.u32 = 0;
		//p = new std::string(filename);
	}
	else if (type == g_editorTypeAnimation)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Animation);
		arg.ptr = new std::string(filename);
	}
	else if (type == g_editorTypeActor)
	{
		fileEntry = vx::FileEntry(filename, vx::FileType::Actor);
		arg.ptr = new std::string(filename);
	}
	else
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_Editor, "Trying to load unknown file type");
		//assert(false);
	}

	vx::lock_guard<vx::mutex> guard(m_editorMutex);
	m_requestedFiles.insert(fileEntry.getSid(), std::make_pair(f, type));

	m_resourceAspect.requestLoadFile(fileEntry, arg);
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

vx::float4a EditorEngine::getRayDir(s32 mouseX, s32 mouseY) const
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
	m_renderAspect->getViewMatrix(&viewMatrix);

	auto inverseViewMatrix = vx::MatrixInverse(viewMatrix);

	vx::float4a ray_world = vx::Vector4Transform(inverseViewMatrix, ray_eye);
	ray_world = vx::normalize3(ray_world);

	return ray_world;
}

vx::StringID EditorEngine::raytraceAgainstStaticMeshes(s32 mouseX, s32 mouseY, vx::float3* hitPosition) const
{
	auto ray_world = getRayDir(mouseX, mouseY);

	vx::float4a cameraPosition;
	m_renderAspect->getCameraPosition(&cameraPosition);

	PhysicsHitData hitData;

	vx::StringID sid;
	if (m_physicsAspect.raycast_staticDynamic(cameraPosition, ray_world, 50.0f, &hitData))
	{
		*hitPosition = hitData.hitPosition;
		sid = hitData.sid;
	}

	return sid;
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

u64 EditorEngine::getMeshInstanceSid(s32 mouseX, s32 mouseY) const
{
	u64 result = 0;
	if (m_pEditorScene)
	{
		vx::float3 p;
		auto sid = raytraceAgainstStaticMeshes(mouseX, mouseY, &p);
		result = sid.value;
	}

	return result;
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

void EditorEngine::setSelectedMeshInstance(u64 sid)
{
	if (sid != 0)
	{
		auto instance = m_pEditorScene->getMeshInstance(vx::StringID(sid));
		if (instance)
		{
			m_renderAspect->setSelectedMeshInstance(instance);
			m_selected.m_type = SelectedType::MeshInstance;
			m_selected.m_item = (void*)instance;
		}
	}
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
		auto mesh = m_pEditorScene->getMesh(vx::StringID(meshSid));
		m_renderAspect->setMeshInstanceMesh(vx::StringID(instanceSid), vx::StringID(meshSid));
		meshInstance->setMeshSid(vx::StringID(meshSid));

		meshInstance->setBounds(mesh->getMesh());

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
		auto material = meshInstance->getMaterial();
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
	m_physicsAspect.addMeshInstance(instance->getMeshInstance());

	m_renderAspect->addMeshInstance(*instance);

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
			if (m_renderAspect->setSelectedMeshInstanceMaterial(sceneMaterial))
			{
				meshInstance->setMaterial(sceneMaterial);
			}
		}
	}
}

void EditorEngine::setMeshInstancePosition(u64 sid, const vx::float3 &p)
{
	if (m_pEditorScene)
	{
		auto instanceSid = vx::StringID(sid);
		m_pEditorScene->setMeshInstancePosition(instanceSid, p);
		auto instance = m_pEditorScene->getMeshInstance(instanceSid);
		auto transform = instance->getTransform();

		m_renderAspect->setSelectedMeshInstanceTransform(transform);
		m_physicsAspect.editorSetStaticMeshInstanceTransform(instance->getMeshInstance(), instanceSid);
	}
}

void EditorEngine::setMeshInstanceRotation(u64 sid, const vx::float3 &rotationDeg)
{
	if (m_pEditorScene)
	{
		auto rotation = vx::degToRad(rotationDeg);
		auto r = vx::loadFloat3(&rotation);

		auto q = vx::quaternionRotationRollPitchYawFromVector(r);

		vx::float4 tmp;
		vx::storeFloat4(&tmp, q);
		
		auto instanceSid = vx::StringID(sid);
		m_pEditorScene->setMeshInstanceRotation(instanceSid, tmp);

		auto instance = m_pEditorScene->getMeshInstance(instanceSid);
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
		vx::quaternionToAxisAngle(vx::loadFloat4(&q), &axis, &angle);
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
	vx::float4a cameraPosition;
	m_renderAspect->getCameraPosition(&cameraPosition);

	Ray ray;
	ray.o.x = static_cast<f32>(cameraPosition.x);
	ray.o.y = static_cast<f32>(cameraPosition.y);
	ray.o.z = static_cast<f32>(cameraPosition.z);
	//vx::storeFloat3(&ray.o, cameraPosition);
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

u32 EditorEngine::createLight()
{
		Graphics::Light light;
		light.m_position = vx::float3(0);
		light.m_falloff = 5.0f;
		light.m_lumen = 100.0f;

		auto index = m_pEditorScene->getLightCount();

		m_selected.m_item = m_pEditorScene->addLight(light);
		m_selected.m_type = SelectedType::Light;

		auto lightCount = index+1;
		auto lights = m_pEditorScene->getLights();

		m_renderAspect->updateLightBuffer(lights, lightCount);

		return index;
}

bool EditorEngine::getLightIndex(s32 mouseX, s32 mouseY, u32* index)
{
	bool result = false;
	if (m_pEditorScene)
	{
		auto ray = getRay(mouseX, mouseY);
		auto light = m_pEditorScene->getLight(ray);

		if (light)
		{
			auto lights = m_pEditorScene->getLights();
			*index = light - lights;
		}

		result = (light != nullptr);
	}

	return result;
}

void EditorEngine::selectLight(u32 index)
{
	m_selected.m_type = SelectedType::Light;
	m_selected.m_item = m_pEditorScene->getLight(index);
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

u32 EditorEngine::getLightCount()
{
	return m_pEditorScene->getLightCount();
}

f32 EditorEngine::getLightLumen(u32 index)
{
	auto light = m_pEditorScene->getLight(index);
	return light->m_lumen;
}

void EditorEngine::setLightLumen(u32 index, f32 lumen)
{
	auto light = m_pEditorScene->getLight(index);
	light->m_lumen = lumen;

	//m_renderAspect->updateLightBuffer(m_pEditorScene->getLights(), m_pEditorScene->getLightCount());
}

f32 EditorEngine::getLightFalloff(u32 index)
{
	auto light = m_pEditorScene->getLight(index);
	return light->m_falloff;
}

void EditorEngine::setLightFalloff(u32 index, f32 falloff)
{
	auto light = m_pEditorScene->getLight(index);
	light->m_falloff = falloff;

	//m_renderAspect->updateLightBuffer(m_pEditorScene->getLights(), m_pEditorScene->getLightCount());
}

void EditorEngine::getLightPosition(u32 index, vx::float3* position)
{
	auto light = m_pEditorScene->getLight(index);
	*position = light->m_position;
}

void EditorEngine::setLightPosition(u32 index, const vx::float3* position)
{
	auto light = m_pEditorScene->getLight(index);
	light->m_position = *position;

	m_renderAspect->updateLightBuffer(m_pEditorScene->getLights(), m_pEditorScene->getLightCount());
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
	if (m_pEditorScene)
	{
		auto &navMesh = m_pEditorScene->getNavMesh();
		m_renderAspect->showNavMesh(b, navMesh, m_navmeshGraph);
	}
}

void EditorEngine::showInfluenceMap(bool b)
{
	if (m_pEditorScene)
	{
		m_renderAspect->showInfluenceMap(b, m_influenceMap);
	}
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

void EditorEngine::setSpawnActor(u32 id, u64 actorSid)
{
	m_pEditorScene->setSpawnActor(id, vx::StringID(actorSid));
}

u32 EditorEngine::getSpawnCount()
{
	return m_pEditorScene->getSpawnCount();
}

u32 EditorEngine::getSpawnId(u32 index)
{
	auto spawns = m_pEditorScene->getSpawns();
	return spawns[index].id;
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

u64 EditorEngine::getSpawnActor(u32 id)
{
	auto spawn = m_pEditorScene->getSpawn(id);
	return spawn->actorSid.value;
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
	auto meshSid = vx::StringID(sid);
	auto meshFile = m_resourceAspect.getMesh(meshSid);
	if (meshFile == nullptr)
	{
		return;
		//VX_ASSERT(false);
		/*auto &meshDataAllocator = m_resourceAspect.getMeshDataAllocator();

		if (m_physicsAspect.setMeshPhysxType(meshFile, (PhsyxMeshType)type, &meshDataAllocator))
		{
			auto fileName = m_resourceAspect.getLoadedFileName(vx::StringID(sid));

			m_fileAspect.requestSaveFile(vx::FileEntry(fileName, vx::FileType::Mesh), meshFile.get());
		}*/
	}

	auto meshManager = m_resourceAspect.getMeshManager();
	auto physxType = (PhsyxMeshType)type;
	std::unique_lock<std::mutex> lock;
	auto dataAllocator = meshManager->lockDataAllocator(&lock);
	if (m_physicsAspect.setMeshPhysxType(meshFile, physxType, dataAllocator))
	{
		vx::Variant arg;
		arg.ptr = meshFile;

		auto meshName = meshManager->getName(meshSid);
		vx::FileEntry fileEntry(meshName, vx::FileType::Mesh);
		m_resourceAspect.requestSaveFile(fileEntry, arg);
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

void EditorEngine::addJoint(const vx::StringID &sid0, const vx::StringID &sid1, const vx::float3 &p0, const vx::float3 &p1)
{
	Joint joint;

	joint.sid0 = sid0;
	joint.sid1 = sid1;
	joint.p0 = p0;
	joint.q0 = {0, 0, 0, 1};
	joint.p1= p1;
	joint.q1 = { 0, 0, 0, 1 };
	joint.type = JointType::Revolute;

	if (sid0.value != 0)
	{
		joint.p0.x = 0;
		joint.p0.y = 0;
		joint.p0.z = 0;
	}

	if (sid1.value != 0)
	{
		joint.p1.x = 0;
		joint.p1.y = 0;
		joint.p1.z = 0;
	}

	if (m_physicsAspect.createJoint(joint))
	{
		puts("created joint");
		m_pEditorScene->addJoint(joint);

		auto jointCount = m_pEditorScene->getJointCount();
		auto joints = m_pEditorScene->getJoints();

		auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();
		m_renderAspect->updateJoints(joints, jointCount, sortedInstances);
	}
}

void EditorEngine::addJoint(const vx::StringID &sid)
{
	auto instance = m_pEditorScene->getMeshInstance(sid);
	if (instance)
	{
		auto &transform = instance->getTransform();

		addJoint(sid, vx::StringID(0), transform.m_translation, transform.m_translation);
	}
}

void EditorEngine::removeJoint(u32 index)
{
	m_pEditorScene->eraseJoint(index);
	auto jointCount = m_pEditorScene->getJointCount();
	auto joints = m_pEditorScene->getJoints();

	auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();
	m_renderAspect->updateJoints(joints, jointCount, sortedInstances);
}

u32 EditorEngine::getJointCount() const
{
	return m_pEditorScene->getJointCount();
}

#include <DirectXMath.h>

void EditorEngine::getJointData(u32 i, vx::float3* p0, vx::float3* q0, vx::float3* p1, vx::float3* q1, u64* sid0, u64* sid1, u32* limitEnabled, f32* limitMin, f32* limitMax) const
{
	auto quaternionToAngles = [](const vx::float4 &q, vx::float3* rotationDeg)
	{
		if (q.x == 0 && q.y == 0 && q.z == 0)
		{
			*rotationDeg = {0, 0, 0};
		}
		else
		{
			auto qq = vx::loadFloat4(&q);

			__m128 axis, axis0;
			f32 angle, angle0;
			DirectX::XMQuaternionToAxisAngle(&axis0, &angle0, qq);
			vx::quaternionToAxisAngle(qq, &axis, &angle);
			axis = vx::normalize3(axis);

			//auto ttt = _mm_mul_ps(axis, _mm_load1_ps(&angle));

			vx::float4a tmpAxis = axis;
			vx::angleAxisToEuler(tmpAxis, angle, rotationDeg);
		}
	};

	auto joints = m_pEditorScene->getJoints();

	auto &joint = joints[i];
	quaternionToAngles(joint.q0, q0);
	quaternionToAngles(joint.q1, q1);
	*p0 = joint.p0;
	*p1 = joint.p1;
	*sid0 = joint.sid0.value;
	*sid1 = joint.sid1.value;
	*limitEnabled = joint.limitEnabled;
	*limitMin = joint.limit.x;
	*limitMax = joint.limit.y;
}

bool EditorEngine::selectJoint(s32 mouseX, s32 mouseY, u32* index)
{
	auto ray = getRay(mouseX, mouseY);
	ray.maxt = 10.0f;

	auto joint = m_pEditorScene->getJoint(ray, index);

	return (joint != nullptr);
}

void EditorEngine::setJointPosition0(u32 index, const vx::float3 &p)
{
	m_pEditorScene->setJointPosition0(index, p);
	auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();
	m_renderAspect->updateJoints(m_pEditorScene->getJoints(), m_pEditorScene->getJointCount(), sortedInstances);
}

void EditorEngine::setJointPosition1(u32 index, const vx::float3 &p)
{
	m_pEditorScene->setJointPosition1(index, p);
	auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();
	m_renderAspect->updateJoints(m_pEditorScene->getJoints(), m_pEditorScene->getJointCount(), sortedInstances);
}

void EditorEngine::setJointBody0(u32 index, u64 sid)
{
	m_pEditorScene->setJointBody0(index, sid);
}

void EditorEngine::setJointBody1(u32 index, u64 sid)
{
	m_pEditorScene->setJointBody1(index, sid);
}

void EditorEngine::setJointRotation0(u32 index, const vx::float3 &q)
{
	auto qq = vx::quaternionRotationRollPitchYawFromVector(vx::degToRad(vx::loadFloat3(&q)));

	vx::float4 tmp;
	vx::storeFloat4(&tmp, qq);

	m_pEditorScene->setJointRotation0(index, tmp);
	auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();
	m_renderAspect->updateJoints(m_pEditorScene->getJoints(), m_pEditorScene->getJointCount(), sortedInstances);
}

void EditorEngine::setJointRotation1(u32 index, const vx::float3 &q)
{
	auto qq = vx::quaternionRotationRollPitchYawFromVector(vx::degToRad(vx::loadFloat3(&q)));

	vx::float4 tmp;
	vx::storeFloat4(&tmp, qq);

	m_pEditorScene->setJointRotation1(index, tmp);
	auto &sortedInstances = m_pEditorScene->getSortedMeshInstances();
	m_renderAspect->updateJoints(m_pEditorScene->getJoints(), m_pEditorScene->getJointCount(), sortedInstances);
}

void EditorEngine::setJointLimit(u32 index, u32 enabled, f32 limitMin, f32 limitMax)
{
	m_pEditorScene->setJointLimit(index, enabled, limitMin, limitMax);
}

u64 EditorEngine::createActor(const char* name, u64 meshSid, u64 materialSid)
{
	vx::FileEntry fileEntry(name, vx::FileType::Actor);

	auto actorFile = new ActorFile(ActorFile::getGlobalVersion());

	auto materialName = m_resourceAspect.getMaterialManager()->getName(vx::StringID(materialSid));
	auto meshName = m_resourceAspect.getMeshManager()->getName(vx::StringID(meshSid));

	const u32 bufferSize = 31;
	auto meshSize = strlen(meshName);
	auto materialSize = strlen(materialName);

	VX_ASSERT(bufferSize >= meshSize && bufferSize >= materialSize);

	char buffer[bufferSize + 1];
	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, meshName, meshSize);
	actorFile->setMesh(buffer);

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, materialName, materialSize);
	actorFile->setMaterial(buffer);

	vx::Variant arg;
	arg.ptr = actorFile;
	m_resourceAspect.requestSaveFile(fileEntry, arg);

	Actor actor;
	actor.m_mesh = meshSid;
	actor.m_material = materialSid;
	m_resourceAspect.addActor(fileEntry.getSid(), std::string(name), actor);

	return fileEntry.getSid().value;
}

const char* EditorEngine::getActorName(u64 sid) const
{
	return m_resourceAspect.getActorManager()->getName(vx::StringID(sid));
}

u32 EditorEngine::getLightGeometryProxyCount() const
{
	return m_pEditorScene->getLightGeometryProxyCount();
}

void EditorEngine::createLightGeometryProxy(const vx::float3 &center, const vx::float3 &halfDim)
{
	AABB bounds;
	bounds.min = center - halfDim;
	bounds.max = center + halfDim;

	m_pEditorScene->addLightGeometryProxy(bounds);

	m_renderAspect->updateLightGeometryProxies(m_pEditorScene->getLightGeometryProxies(), m_pEditorScene->getLightGeometryProxyCount());
}

void EditorEngine::setLightGeometryProxyBounds(u32 index, const vx::float3 &center, const vx::float3 &halfDim)
{
	AABB bounds;
	bounds.min = center - halfDim;
	bounds.max = center + halfDim;
	m_pEditorScene->setLightGeometryProxyBounds(index, bounds);

	m_renderAspect->updateLightGeometryProxies(m_pEditorScene->getLightGeometryProxies(), m_pEditorScene->getLightGeometryProxyCount());
}

void EditorEngine::getLightGeometryProxyBounds(u32 index, vx::float3* center, vx::float3* halfDimOut) const
{
	auto ptr = m_pEditorScene->getLightGeometryProxies();

	auto bounds = ptr[index].m_bounds;

	auto halfDim = (bounds.max - bounds.min) * 0.5f;

	*center = bounds.min + halfDim;
	*halfDimOut = halfDim;
}

u32 EditorEngine::getLightGeometryProxyLightCount(u32 index) const
{
	auto ptr = m_pEditorScene->getLightGeometryProxies();

	return ptr[index].m_lightCount;
}

void EditorEngine::testLightGeometryProxies()
{
	Editor::TestLightGeometryProxiesDesc desc;
	desc.lightCount = m_pEditorScene->getLightCount();
	desc.lights = m_pEditorScene->getLights();
	desc.proxies = m_pEditorScene->getLightGeometryProxies();
	desc.proxyCount = m_pEditorScene->getLightGeometryProxyCount();

	m_renderAspect->testLightGeometryProxies(desc);
}