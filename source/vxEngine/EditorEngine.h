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
#include "Editor.h"
#include "EventListener.h"
#include "InfluenceMap.h"

enum class SelectedType{ None, MeshInstance, NavMeshVertex };

class EditorEngine : public EventListener
{
	static U32 s_editorTypeMesh;
	static U32 s_editorTypeMaterial;
	static U32 s_editorTypeScene;

	struct SelectedNavMeshVertices
	{
		U32 m_vertices[3];
		U8 m_count;
	};

	struct Selected
	{
		SelectedType m_type;
		union
		{
			SelectedNavMeshVertices m_navMeshVertices;
			void* m_item{ nullptr };
		};
	};

	EventManager m_eventManager;
	PhysicsAspect m_physicsAspect;
	EditorRenderAspect m_renderAspect;
	EditorScene* m_pEditorScene{ nullptr };
	InfluenceMap m_influenceMap;
	std::mutex m_editorMutex;
	VX_ALIGN(64) struct
	{
		FileAspect m_fileAspect;
		std::atomic_uint m_bRunFileThread;
	};
	Selected m_selected;
	Editor::WaypointManager m_waypointManager;
	vx::thread m_fileAspectThread;
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_scratchAllocator;
	U32 m_shutdown{ 0 };
	Memory m_memory;
	vx::uint2 m_resolution;
	HWND m_panel;

	vx::sorted_vector<vx::StringID64, std::pair<Editor::LoadFileCallback, U32>> m_requestedFiles;

	// calls the callback provided by editor_loadFile
	void call_editorCallback(const vx::StringID64 &sid);

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void handleFileEvent(const Event &evt);

	vx::float4a getRayDir(I32 mouseX, I32 mouseY);

	MeshInstance* raytraceAgainstStaticMeshes(I32 mouseX, I32 mouseY, vx::float3* hitPosition);

	void createStateMachine();

	U32 getSelectedNavMeshVertex(I32 mouseX, I32 mouseY);

	void buildNavGraph();

public:
	EditorEngine();
	~EditorEngine();

	bool initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, EditorScene* pScene);
	void shutdownEditor();

	static void editor_setTypes(U32 mesh, U32 material, U32 scene);

	void editor_saveScene(const char* name);

	void editor_start();
	void editor_render(F32 dt);
	void editor_loadFile(const char *filename, U32 type, Editor::LoadFileCallback f);

	void editor_moveCamera(F32 dirX, F32 dirY, F32 dirZ);
	void editor_rotateCamera(F32 dirX, F32 dirY, F32 dirZ);

	void stop();

	void handleEvent(const Event &evt);

	void requestLoadFile(const FileEntry &fileEntry, void* p);

	void updateSelectedMeshInstanceTransform(const vx::float3 &p);

	void addWaypoint(const vx::float3 &p);


	void setSelectedNavMeshVertexPosition(const vx::float3 &position);
	vx::float3 getSelectedNavMeshVertexPosition() const;

	bool selectMesh(I32 mouseX, I32 mouseY);
	void deselectMesh();

	bool addNavMeshVertex(I32 mouseX, I32 mouseY);
	void deleteSelectedNavMeshVertex();
	bool selectNavMeshVertex(I32 mouseX, I32 mouseY);
	bool multiSelectNavMeshVertex(I32 mouseX, I32 mouseY);
	void deselectNavMeshVertex();
	bool createNavMeshTriangleFromSelectedVertices();

	SelectedType getSelectedItemType() const;
	EditorScene* getEditorScene() const;
};