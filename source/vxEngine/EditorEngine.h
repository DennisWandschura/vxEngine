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
#include "EditorPhysicsAspect.h"
#include <vxResourceAspect/FileAspect.h>
#include "thread.h"
#include "memory.h"
#include "LevelEditor.h"
#include "Editor.h"
#include <vxEngineLib/EventListener.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/EditorRenderAspectInterface.h>
#include "TaskManager.h"

enum class SelectedType{ None, MeshInstance, NavMeshVertex, Light };

class EditorEngine : public vx::EventListener
{
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
	Editor::PhysicsAspect m_physicsAspect;
	Editor::RenderAspectInterface* m_renderAspect;
	Editor::Scene* m_pEditorScene{ nullptr };
	InfluenceMap m_influenceMap;
	vx::mutex m_editorMutex;
	struct VX_ALIGN(64)
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
	HMODULE m_renderAspectDll;
	DestroyEditorRenderAspectFunction m_destroyFn;
	bool m_previousSceneLoaded;

	vx::sorted_vector<vx::StringID, std::pair<Editor::LoadFileCallback, u32>> m_requestedFiles;

	// calls the callback provided by editor_loadFile
	bool call_editorCallback(const vx::StringID &sid);

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void handleFileEvent(const vx::Event &evt);

	vx::float4a getRayDir(s32 mouseX, s32 mouseY);

	vx::StringID raytraceAgainstStaticMeshes(s32 mouseX, s32 mouseY, vx::float3* hitPosition);

	bool createRenderAspectGL(const std::string &dataDir, const RenderAspectDescription &desc);

	Ray getRay(s32 mouseX, s32 mouseY);
	u32 getSelectedNavMeshVertex(s32 mouseX, s32 mouseY);

	void buildNavGraph();

public:
	EditorEngine();
	~EditorEngine();

	bool initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, Editor::Scene* pScene);
	void shutdownEditor();

	static void editor_setTypes(u32 mesh, u32 material, u32 scene, u32 fbx, u32 animation);

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
	const char* getMeshInstanceName(const vx::StringID &sid) const;
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

	vx::StringID createMeshInstance();
	void removeMeshInstance(u64 sid);

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

	void setMeshInstanceAnimation(u64 instanceSid, u64 animSid);
	u64 getMeshInstanceAnimation(u64 instanceSid);

	void createLight();
	bool selectLight(s32 mouseX, s32 mouseY);
	void deselectLight();
	void getSelectLightPosition(vx::float3* position) const;
	void setSelectLightPosition(const vx::float3 &position);
	f32 getSelectLightFalloff() const;
	void setSelectLightFalloff(f32 falloff);
	f32 getSelectLightLumen() const;
	void setSelectLightLumen(f32 lumen);

	SelectedType getSelectedItemType() const;
	Editor::Scene* getEditorScene() const;

	void showNavmesh(bool b);
	void showInfluenceMap(bool b);

	bool addWaypoint(s32 mouseX, s32 mouseY, vx::float3* position);
	void addWaypoint(const vx::float3 &position);
	void removeWaypoint(const vx::float3 &position);

	void addSpawn();
	bool selectSpawn(s32 mouseX, s32 mouseY, u32* id);
	void getSpawnPosition(u32 id, vx::float3* position) const;
	u32 getSpawnType(u32 id) const;

	void setSpawnPosition(u32 id, const vx::float3 &position);
	void setSpawnType(u32 id, u32 type);

	u32 getMeshCount() const;
	const char* getMeshName(u32 i) const;
	u64 getMeshSid(u32 i) const;

	u32 getMaterialCount() const;
	const char* getMaterialNameIndex(u32 i) const;
	const char* getMaterialName(u64 sid) const;
	u64 getMaterialSid(u32 i) const;

	u32 getAnimationCount() const;
	const char* getAnimationNameIndex(u32 i) const;
	u64 getAnimationSidIndex(u32 i) const;

	u32 getMeshPhysxType(u64 sid) const;
	void setMeshPhysxType(u64 sid, u32 type);

	u32 getMeshInstanceRigidBodyType(u64 sid) const;
	void setMeshInstanceRigidBodyType(u64 sid, u32 type);

	u32 getJointCount() const;
	void getJointData(u32 i, vx::float3* p0, vx::float3* p1, u64* sid0, u64* sid1) const;
	void addJoint(const vx::StringID &sid0, const vx::StringID &sid1, const vx::float3 &p0, const vx::float3 &p1);
	void addJoint(const vx::StringID &sid0);
	void removeJoint(u32 index);
	bool selectJoint(s32 mouseX, s32 mouseY, u32* index);
	void setJointPosition0(u32 index, const vx::float3 &p);
	void setJointPosition1(u32 index, const vx::float3 &p);
	void setJointBody0(u32 index, u64 sid);
	void setJointBody1(u32 index, u64 sid);
};