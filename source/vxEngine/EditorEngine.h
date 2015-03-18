#pragma once

class EditorScene;

#include "EventManager.h"
#include "SystemAspect.h"
#include "EditorRenderAspect.h"
#include "PhysicsAspect.h"
#include "FileAspect.h"
#include "thread.h"
#include "memory.h"
#include "LevelEditor.h"
#include "EditorWaypointManager.h"

namespace Editor
{
	typedef void(*LoadFileCallback)(UINT64, unsigned int);
}

class EditorEngine
{
	static U32 s_editorTypeMesh;
	static U32 s_editorTypeMaterial;
	static U32 s_editorTypeScene;

	EventManager m_eventManager;
	PhysicsAspect m_physicsAspect;
	EditorRenderAspect m_renderAspect;
	EditorScene* m_pEditorScene{ nullptr };
	std::mutex m_editorMutex;
	VX_ALIGN(64) struct
	{
		FileAspect m_fileAspect;
		std::atomic_uint m_bRunFileThread;
	};
	Editor::WaypointManager m_waypointManager;
	vx::thread m_fileAspectThread;
	vx::StackAllocator m_allocator;
	U32 m_shutdown{ 0 };
	Memory m_memory;
	vx::uint2 m_resolution;

	vx::sorted_vector<vx::StringID64, std::pair<Editor::LoadFileCallback, U32>> m_requestedFiles;

	// calls the callback provided by editor_loadFile
	void call_editorCallback(const vx::StringID64 sid);

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void handleFileEvent(const Event &evt);

public:
	explicit EditorEngine(Logfile &logfile);
	~EditorEngine();

	bool initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, EditorScene* pScene);
	void shutdownEditor();

	static void editor_setTypes(U32 mesh, U32 material, U32 scene);

	void editor_saveScene(const char* name);

	void editor_start();
	void editor_render(F32 dt);
	void editor_loadFile(const char *filename, U32 type, Editor::LoadFileCallback f);

	// returns 1 on success, 0 on failure
	U8 editor_addMeshInstance(const vx::StringID64 instanceSid, const vx::StringID64 meshSid, const vx::StringID64 materialSid, const vx::Transform &transform);

	U32 editor_getTransform(const vx::StringID64 instanceSid, vx::float3 &translation, vx::float3 &rotation, F32 &scaling);
	void editor_updateTranslation(const vx::StringID64 instanceSid, const vx::float3 &translation);

	void editor_moveCamera(F32 dirX, F32 dirY, F32 dirZ);
	void editor_rotateCamera(F32 dirX, F32 dirY, F32 dirZ);

	void stop();

	void handleEvent(const Event &evt);

	void requestLoadFile(const FileEntry &fileEntry, void* p);

	U8 raytraceMouse(I32 x, I32 y, vx::float3* p);

	void addWaypoint(const vx::float3 &p);
};