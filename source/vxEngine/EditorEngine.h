#pragma once
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

namespace Editor
{
	class Scene;
}

#include <vxEngineLib/EventManager.h>
#include "SystemAspect.h"
#include "EditorRenderAspect.h"
#include "PhysicsAspect.h"
#include <vxResourceAspect/FileAspect.h>
#include "thread.h"
#include "memory.h"
#include "LevelEditor.h"
#include "Editor.h"
#include <vxEngineLib/EventListener.h>
#include "InfluenceMap.h"

enum class SelectedType{ None, MeshInstance, NavMeshVertex, Light };

class EditorEngine : public vx::EventListener
{
	static u32 s_editorTypeMesh;
	static u32 s_editorTypeMaterial;
	static u32 s_editorTypeScene;

	struct SelectedNavMeshVertices
	{
		u32 m_vertices[3];
		u8 m_count;
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

	vx::EventManager m_eventManager;
	PhysicsAspect m_physicsAspect;
	EditorRenderAspect m_renderAspect;
	Editor::Scene* m_pEditorScene{ nullptr };
	InfluenceMap m_influenceMap;
	std::mutex m_editorMutex;
	VX_ALIGN(64) struct
	{
		FileAspect m_fileAspect;
		std::atomic_uint m_bRunFileThread;
	};
	Selected m_selected;
	vx::thread m_fileAspectThread;
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_scratchAllocator;
	u32 m_shutdown{ 0 };
	Memory m_memory;
	vx::uint2 m_resolution;
	HWND m_panel;

	vx::sorted_vector<vx::StringID, std::pair<Editor::LoadFileCallback, u32>> m_requestedFiles;

	// calls the callback provided by editor_loadFile
	bool call_editorCallback(const vx::StringID &sid);

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void handleFileEvent(const vx::Event &evt);

	vx::float4a getRayDir(s32 mouseX, s32 mouseY);

	vx::StringID raytraceAgainstStaticMeshes(s32 mouseX, s32 mouseY, vx::float3* hitPosition);

	void createStateMachine();

	Ray getRay(s32 mouseX, s32 mouseY);
	u32 getSelectedNavMeshVertex(s32 mouseX, s32 mouseY);

	void buildNavGraph();

	void addMesh(const vx::StringID &sid);

public:
	EditorEngine();
	~EditorEngine();

	bool initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, Editor::Scene* pScene);
	void shutdownEditor();

	static void editor_setTypes(u32 mesh, u32 material, u32 scene);

	void editor_saveScene(const char* name);

	void editor_start();
	void editor_render();
	void editor_loadFile(const char *filename, u32 type, Editor::LoadFileCallback f);

	void editor_moveCamera(f32 dirX, f32 dirY, f32 dirZ);
	void editor_rotateCamera(f32 dirX, f32 dirY, f32 dirZ);

	void stop();

	void handleEvent(const vx::Event &evt);

	void requestLoadFile(const vx::FileEntry &fileEntry, void* p);

	void setSelectedNavMeshVertexPosition(const vx::float3 &position);
	bool getSelectedNavMeshVertexPosition(vx::float3* p) const;

	u32 getMeshInstanceCount() const;
	const char* getMeshInstanceName(u32 i) const;
	u64 getMeshInstanceSid(u32 i) const;
	const char* getSelectedMeshInstanceName() const;

	u64 getSelectedMeshInstanceSid() const;
	void setMeshInstanceMeshSid(u64 instanceSid, u64 meshSid);

	u64 getMeshInstanceMeshSid(u64 instanceSid) const;
	u64 getMeshInstanceMaterialSid(u64 instanceSid) const;
	void getMeshInstancePosition(u64 sid, vx::float3* position);
	bool selectMeshInstance(s32 mouseX, s32 mouseY);
	bool selectMeshInstance(u32 i);
	bool selectMeshInstance(u64 sid);
	u64 deselectMeshInstance();

	void createMeshInstance();
	void removeSelectedMeshInstance();

	void setMeshInstancePosition(u64 sid, const vx::float3 &p);
	void setMeshInstanceRotation(u64 sid, const vx::float3 &rotationDeg);
	void getMeshInstanceRotation(u64 sid, vx::float3* rotationDeg) const;
	void setMeshInstanceMaterial(u64 instanceSid, u64 materialSid);
	bool setMeshInstanceName(u64 sid, const char* name);

	bool addNavMeshVertex(s32 mouseX, s32 mouseY, vx::float3* position);
	void removeNavMeshVertex(const vx::float3 &position);
	void removeSelectedNavMeshVertex();
	bool selectNavMeshVertex(s32 mouseX, s32 mouseY);
	bool selectNavMeshVertexIndex(u32 index);
	bool selectNavMeshVertexPosition(const vx::float3 &position);
	bool multiSelectNavMeshVertex(s32 mouseX, s32 mouseY);
	u32 deselectNavMeshVertex();
	bool createNavMeshTriangleFromSelectedVertices(vx::uint3* selected);
	void createNavMeshTriangleFromIndices(const vx::uint3 &indices);
	void removeNavMeshTriangle();
	u32 getSelectedNavMeshCount() const;

	void createLight();
	bool selectLight(s32 mouseX, s32 mouseY);
	void deselectLight();
	void getSelectLightPosition(vx::float3* position);
	void setSelectLightPosition(const vx::float3 &position);

	SelectedType getSelectedItemType() const;
	Editor::Scene* getEditorScene() const;

	void showNavmesh(bool b);
	void showInfluenceMap(bool b);

	bool addWaypoint(s32 mouseX, s32 mouseY, vx::float3* position);
	void addWaypoint(const vx::float3 &position);
	void removeWaypoint(const vx::float3 &position);

	u32 getMeshCount() const;
	const char* getMeshName(u32 i) const;
	u64 getMeshSid(u32 i) const;

	u32 getMaterialCount() const;
	const char* getMaterialName(u32 i) const;
	u64 getMaterialSid(u32 i) const;
};