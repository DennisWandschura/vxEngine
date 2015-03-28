#include "EditorEngine.h"
#include "enums.h"
#include "Event.h"
#include "EventTypes.h"

U32 EditorEngine::s_editorTypeMesh{ -1 };
U32 EditorEngine::s_editorTypeMaterial{ -1 };
U32 EditorEngine::s_editorTypeScene{ -1 };

EditorEngine::EditorEngine()
	:m_eventManager(),
	m_physicsAspect(m_fileAspect),
	m_renderAspect(m_fileAspect),
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

	if (!m_fileAspect.initialize(&m_allocator, dataDir))
		return false;

	return true;
}

bool EditorEngine::initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, EditorScene* pScene)
{
	const std::string dataDir("../../game/data/");
	m_pEditorScene = pScene;
	m_resolution = resolution;

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

	if (!m_physicsAspect.initialize())
	{
		return false;
	}

	m_eventManager.initialize();

	m_eventManager.registerListener(&m_renderAspect, 1);
	m_eventManager.registerListener(&m_physicsAspect, 1);

	//m_bRun = 1;
	m_bRunFileThread.store(1);
	m_shutdown = 0;

	return true;
}

void EditorEngine::shutdownEditor()
{
	m_fileAspectThread.join();
	m_fileAspect.shutdown();

	m_physicsAspect.shutdown();
	//m_renderAspect.shutdown();

	m_shutdown = 1;
	m_allocator.release();
}

void EditorEngine::stop()
{
	//m_bRun = 0;
	m_bRunFileThread.store(0);
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
		std::string* pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);
		auto sid = evt.arg1.sid;

		auto pMesh = m_fileAspect.getMesh(sid);
		m_renderAspect.editor_addMesh(sid, pStr->c_str(), pMesh);

		call_editorCallback(sid);

		delete(pStr);
	}break;
	case::FileEvent::Texture_Loaded:
		break;
	case::FileEvent::Material_Loaded:
	{
		std::string* pStr = reinterpret_cast<std::string*>(evt.arg2.ptr);
		auto sid = evt.arg1.sid;

		auto pMaterial = m_fileAspect.getMaterial(sid);
		VX_ASSERT(pMaterial != nullptr);
		m_renderAspect.editor_addMaterial(sid, pStr->c_str(), pMaterial);

		call_editorCallback(sid);

		delete(pStr);
	}break;
	case FileEvent::Scene_Loaded:
		call_editorCallback(evt.arg2.sid);
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
	//m_renderAspect.editor_setScene(m_pEditorScene);
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

U8 EditorEngine::editor_addMeshInstance(const vx::StringID64 instanceSid, const vx::StringID64 meshSid, const vx::StringID64 materialSid, const vx::Transform &transform)
{
	return m_renderAspect.editor_addMeshInstance(instanceSid, meshSid, materialSid, transform);
}

U32 EditorEngine::editor_getTransform(const vx::StringID64 instanceSid, vx::float3 &translation, vx::float3 &rotation, F32 &scaling)
{
	return m_renderAspect.editor_getTransform(instanceSid, translation, rotation, scaling);
}

void EditorEngine::editor_updateTranslation(const vx::StringID64 instanceSid, const vx::float3 &translation)
{
	m_renderAspect.editor_updateTranslation(instanceSid, translation);
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

	//auto v = DirectX::XMQuaternionRotationRollPitchYaw(y, x, 0);

	m_renderAspect.editor_rotateCamera(v);
}

void EditorEngine::call_editorCallback(const vx::StringID64 sid)
{
	std::lock_guard<std::mutex> guard(m_editorMutex);
	auto it = m_requestedFiles.find(sid);
	(*it->first)(sid.m_value, it->second);
	m_requestedFiles.erase(it);
}

U8 EditorEngine::raytraceMouse(I32 x, I32 y, vx::float3* p)
{
	// bring x, y to range -1 ... 1
	F32 ndc_x = F32(x) / m_resolution.x;
	F32 ndc_y = F32(y) / m_resolution.y;

	ndc_x = ndc_x * 2.0f - 1.0f;
	ndc_y = 1.0f - ndc_y * 2.0f;

	vx::mat4 viewMatrix;
	m_renderAspect.getCamera().getViewMatrix(viewMatrix);
	auto cameraPosition = m_renderAspect.getCamera().getPosition();
	auto inverseViewMatrix = vx::MatrixInverse(viewMatrix);

	vx::mat4 projMatrix;
	m_renderAspect.getProjectionMatrix(&projMatrix);
	projMatrix = vx::MatrixInverse(projMatrix);

	vx::float4a ray_clip(ndc_x, ndc_y, -1, 1);
	vx::float4a ray_eye = vx::Vector4Transform(projMatrix, ray_clip);
	ray_eye.z = -1.0f;
	ray_eye.w = 0.0f;

	vx::float4a ray_world = vx::Vector4Transform(inverseViewMatrix, ray_eye);
	ray_world = vx::Vector3Normalize(ray_world);

	auto result = m_physicsAspect.raycast_static(vx::float3(cameraPosition.f[0], cameraPosition.f[1], cameraPosition.f[2]), vx::float3(ray_world.x, ray_world.y, ray_world.z), 25.0f, p);
	if (result != 0)
	{
		m_renderAspect.editor_updateMouseHit(*p);
	}

	return result;
}

void EditorEngine::addWaypoint(const vx::float3 &p)
{
	m_waypointManager.addWaypoint(p, &m_renderAspect);
}