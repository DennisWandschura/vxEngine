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

#include <vxEngineLib/MessageManager.h>
#include "SystemAspect.h"
#include "EditorPhysicsAspect.h"
#include <vxResourceAspect/ResourceAspect.h>
#include "memory.h"
#include "LevelEditor.h"
#include "Editor.h"
#include <vxEngineLib/MessageListener.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/Graphics/EditorRenderAspectInterface.h>
#include <vxEngineLib/TaskManager.h>
#include <vxEngineLib/NavMeshGraph.h>

enum class SelectedType{ None, MeshInstance, NavMeshVertex, Light };

class EditorEngine : public vx::MessageListener
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

	vx::MessageManager m_msgManager;
	Editor::PhysicsAspect m_physicsAspect;
	Editor::RenderAspectInterface* m_renderAspect;
	Editor::Scene* m_pEditorScene{ nullptr };
	InfluenceMap m_influenceMap;
	NavMeshGraph m_navmeshGraph;
	vx::TaskManager m_taskManager;
	vx::mutex m_editorMutex;
	ResourceAspect m_resourceAspect;
	Selected m_selected;
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_scratchAllocator;
	u32 m_shutdown{ 0 };
	Memory m_memory;
	vx::uint2 m_resolution;
	HWND m_panel;
	HMODULE m_renderAspectDll;
	DestroyEditorRenderAspectFunction m_destroyFn;
	bool m_previousSceneLoaded;
	std::thread m_taskManagerThread;

	vx::sorted_vector<vx::StringID, std::pair<Editor::LoadFileCallback, u32>> m_requestedFiles;

	// calls the callback provided by editor_loadFile
	bool call_editorCallback(const vx::StringID &sid);

	bool initializeImpl(const std::string &dataDir, bool flipTextures);

	void handleFileEvent(const vx::Message &evt);

	vx::float4a getRayDir(s32 mouseX, s32 mouseY) const;

	vx::StringID raytraceAgainstStaticMeshes(s32 mouseX, s32 mouseY, vx::float3* hitPosition) const;

	bool createRenderAspectGL(const std::string &dataDir, const RenderAspectDescription &desc, AbortSignalHandlerFun signalHandlerFn);

	Ray getRay(s32 mouseX, s32 mouseY);
	u32 getSelectedNavMeshVertex(s32 mouseX, s32 mouseY);

	void buildNavGraph();

public:
	EditorEngine();
	~EditorEngine();

	bool initializeEditor(HWND panel, HWND tmp, const vx::uint2 &resolution, AbortSignalHandlerFun signalHandlerFn, Editor::Scene* pScene, Logfile* logfile);
	void shutdownEditor();

	static void editor_setTypes(u32 mesh, u32 material, u32 scene, u32 fbx, u32 animation, u32 typeActor);

	void editor_saveScene(const char* name);

	void update(f32 dt);
	void editor_loadFile(const char *filename, u32 type, Editor::LoadFileCallback f, vx::Variant arg);

	void editor_moveCamera(f32 dirX, f32 dirY, f32 dirZ);
	void editor_rotateCamera(f32 dirX, f32 dirY, f32 dirZ);

	void stop();

	void handleMessage(const vx::Message &evt);

	void requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg);

	void setSelectedNavMeshVertexPosition(const vx::float3 &position);
	bool getSelectedNavMeshVertexPosition(vx::float3* p) const;

	u32 getMeshInstanceCount() const;
	const char* getMeshInstanceName(u32 i) const;
	const char* getMeshInstanceName(const vx::StringID &sid) const;
	u64 getMeshInstanceSid(u32 i) const;
	u64 getMeshInstanceSid(s32 mouseX, s32 mouseY) const;
	const char* getSelectedMeshInstanceName() const;

	void setSelectedMeshInstance(u64 sid);
	u64 getSelectedMeshInstanceSid() const;
	void setMeshInstanceMeshSid(u64 instanceSid, u64 meshSid);

	u64 getMeshInstanceMeshSid(u64 instanceSid) const;
	u64 getMeshInstanceMaterialSid(u64 instanceSid) const;
	void getMeshInstancePosition(u64 sid, vx::float3* position);
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

	u32 createLight();
	bool getLightIndex(s32 mouseX, s32 mouseY, u32* index);
	void selectLight(u32 index);
	void deselectLight();
	u32 getLightCount();
	f32 getLightLumen(u32 index);
	void setLightLumen(u32 index, f32 lumen);
	f32 getLightFalloff(u32 index);
	void setLightFalloff(u32 index, f32 falloff);
	void getLightPosition(u32 index, vx::float3* position);
	void setLightPosition(u32 index, const vx::float3* position);

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
	void setSpawnActor(u32 id, u64 actorSid);
	u32 getSpawnCount();
	u32 getSpawnId(u32 index);
	u64 getSpawnActor(u32 id);

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
	void getJointData(u32 i, vx::float3* p0, vx::float3* q0, vx::float3* p1, vx::float3* q1, u64* sid0, u64* sid1, u32* limitEnabled, f32* limitMin, f32* limitMax) const;
	void addJoint(const vx::StringID &sid0, const vx::StringID &sid1, const vx::float3 &p0, const vx::float3 &p1);
	void addJoint(const vx::StringID &sid0);
	void removeJoint(u32 index);
	bool selectJoint(s32 mouseX, s32 mouseY, u32* index);
	void setJointPosition0(u32 index, const vx::float3 &p);
	void setJointPosition1(u32 index, const vx::float3 &p);
	void setJointBody0(u32 index, u64 sid);
	void setJointBody1(u32 index, u64 sid);
	void setJointRotation0(u32 index, const vx::float3 &q);
	void setJointRotation1(u32 index, const vx::float3 &q);
	void setJointLimit(u32 index, u32 enabled, f32 limitMin, f32 limitMax);

	u64 createActor(const char* name, u64 meshSid, u64 materialSid);
	const char* getActorName(u64 sid) const;

	u32 getLightGeometryProxyCount() const;
	void createLightGeometryProxy(const vx::float3 &center, const vx::float3 &halfDim);
	void setLightGeometryProxyBounds(u32 index, const vx::float3 &center, const vx::float3 &halfDim);
	void getLightGeometryProxyBounds(u32 index, vx::float3* center, vx::float3* halfDim) const;
	u32 getLightGeometryProxyLightCount(u32 index) const;
	void testLightGeometryProxies();
};