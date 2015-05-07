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

enum class SelectedType{ None, MeshInstance, NavMeshVertex, Light };

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

	vx::sorted_vector<vx::StringID, std::pair<Editor::LoadFileCallback, U32>> m_requestedFiles;

	// calls the callback provided by editor_loadFile
	void call_editorCallback(vx::StringID sid);

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void handleFileEvent(const Event &evt);

	vx::float4a getRayDir(I32 mouseX, I32 mouseY);

	MeshInstance* raytraceAgainstStaticMeshes(I32 mouseX, I32 mouseY, vx::float3* hitPosition);

	void createStateMachine();

	Ray getRay(I32 mouseX, I32 mouseY);
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

	void createLight();
	bool selectLight(I32 mouseX, I32 mouseY);
	void deselectLight();
	void getSelectLightPosition(vx::float3* position);
	void setSelectLightPosition(const vx::float3 &position);

	SelectedType getSelectedItemType() const;
	EditorScene* getEditorScene() const;

	void showNavmesh(bool b);
	void showInfluenceMap(bool b);
};