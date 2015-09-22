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
#include "LevelEditor.h"
#include "EditorEngine.h"
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Transform.h>
#include <vxEngineLib/EditorScene.h>
#include <vxLib/File/FileHandle.h>
#include <vxEngineLib/SmallObject.h>
#include <vxEngineLib/Logfile.h>
#include <csignal>

#ifdef _VX_EDITOR
namespace Editor
{
	void signalHandler(int signal)
	{
		if (signal == SIGABRT)
		{
			puts("SIGABRT received");
			//shutdown();
		}
		else
		{
			printf("Unexpected signal %d  received\n", signal);
		}
		std::exit(EXIT_FAILURE);
	}

	struct Editor
	{
		//Timer clock;
		Logfile logfile;
		EditorEngine engine;
		Editor::Scene scene;

		Editor()
			:engine(),
			scene()
		{
		}
	};

	Editor* g_pEditor{ nullptr };
	void* g_pMemory{ nullptr };
	vx::aligned_ptr<SmallObjAllocator> g_alloc{};
	LARGE_INTEGER g_last{};
	f64 g_invFrequency{ 1.0 };
	const u32 g_hz = 40u;
	const f32 g_dt = 1.0f / g_hz;

	BSTR ANSItoBSTR(const char* input)
	{
		BSTR result = NULL;
		int lenA = lstrlenA(input);
		int lenW = ::MultiByteToWideChar(CP_ACP, 0, input, lenA, NULL, 0);
		if (lenW > 0)
		{
			result = ::SysAllocStringLen(0, lenW);
			::MultiByteToWideChar(CP_ACP, 0, input, lenA, result, lenW);
		}
		return result;
	}

	template<class T>
	void destroy(T *p)
	{
		p->~T();
	}

	bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, u32 panelSizeX, u32 panelSizeY, u32 typeMesh, u32 typeMaterial, u32 typeScene, u32 typeFbx, u32 typeAnimation)
	{
		EditorEngine::editor_setTypes(typeMesh, typeMaterial, typeScene, typeFbx, typeAnimation);

		g_pMemory = ::operator new(sizeof(Editor));
		if (!g_pMemory)
			return false;

		auto pEditor = new(g_pMemory)Editor();

		g_alloc = vx::aligned_ptr<SmallObjAllocator>(1 KBYTE);
		SmallObject::setAllocator(g_alloc.get());
		Task::setAllocator(g_alloc.get());
		Event::setAllocator(g_alloc.get());

		//pEditor->logfile.create("editor_log.xml");
		pEditor->logfile.create("log.txt");
		if (!pEditor->engine.initializeEditor((HWND)hwndPanel, (HWND)hwndTmp, vx::uint2(panelSizeX, panelSizeY), signalHandler, &pEditor->scene, &pEditor->logfile))
			return false;

		g_pEditor = pEditor;

		//g_pEditor->engine.editor_start();

		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		g_invFrequency = 1.0 / frequency.QuadPart;
		QueryPerformanceCounter(&g_last);

		return true;
	}

	void shutdownEditor()
	{
		if (g_pEditor)
		{

			g_pEditor->engine.stop();
			g_pEditor->engine.shutdownEditor();
			destroy(g_pEditor);

			g_pEditor->logfile.close();
		}

		if (g_alloc.get())
		{
			g_alloc.reset();
			Task::setAllocator(nullptr);
			Event::setAllocator(nullptr);
		}

		::operator delete(g_pMemory);
		g_pMemory = nullptr;
	}

	void frame()
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		auto frameTicks = (current.QuadPart - g_last.QuadPart) * 1000;
		f32 frameTime = frameTicks * g_invFrequency * 0.001f;
		frameTime = fminf(frameTime, g_dt);

		g_last = current;

		g_pEditor->engine.update(frameTime);

	}

	void loadFile(const char *filename, unsigned int type, LoadFileCallback f)
	{
		g_pEditor->engine.editor_loadFile(filename, type, f, vx::Variant());
	}

	u64 getSid(const char *str)
	{
		auto tmp =vx::FileHandle(str);

		return tmp.m_sid.value;
	}

	void saveScene(const char* name)
	{
		g_pEditor->engine.editor_saveScene(name);
	}

	void moveCamera(f32 dirX, f32 dirY, f32 dirZ)
	{
		g_pEditor->engine.editor_moveCamera(dirX, dirY, dirZ);
	}

	void rotateCamera(f32 dirX, f32 dirY, f32 dirZ)
	{
		g_pEditor->engine.editor_rotateCamera(dirX, dirY, dirZ);
	}

	bool selectNavMeshVertex(s32 x, s32 y)
	{
		return g_pEditor->engine.selectNavMeshVertex(x, y);
	}

	bool selectNavMeshVertexIndex(u32 index)
	{
		return g_pEditor->engine.selectNavMeshVertexIndex(index);
	}

	bool selectNavMeshVertexPosition(const vx::float3 &position)
	{
		return g_pEditor->engine.selectNavMeshVertexPosition(position);
	}

	bool multiSelectNavMeshVertex(s32 mouseX, s32 mouseY)
	{
		return g_pEditor->engine.multiSelectNavMeshVertex(mouseX, mouseY);
	}

	u32 deselectNavMeshVertex()
	{
		return g_pEditor->engine.deselectNavMeshVertex();
	}

	bool createNavMeshTriangleFromSelectedVertices(vx::uint3* selected)
	{
		return g_pEditor->engine.createNavMeshTriangleFromSelectedVertices(selected);
	}

	void createNavMeshTriangleFromIndices(const vx::uint3 &indices)
	{
		return g_pEditor->engine.createNavMeshTriangleFromIndices(indices);
	}

	void removeNavMeshTriangle()
	{
		g_pEditor->engine.removeNavMeshTriangle();
	}

	bool addNavMeshVertex(s32 x, s32 y, vx::float3* position)
	{
		return g_pEditor->engine.addNavMeshVertex(x, y, position);
	}

	void removeNavMeshVertex(const vx::float3 &position)
	{
		g_pEditor->engine.removeNavMeshVertex(position);
	}

	void removeNavMeshVertex()
	{
		g_pEditor->engine.removeSelectedNavMeshVertex();
	}

	bool getSelectNavMeshVertexPosition(vx::float3* position)
	{
		return g_pEditor->engine.getSelectedNavMeshVertexPosition(position);
	}

	void setSelectNavMeshVertexPosition(const vx::float3 &position)
	{
		g_pEditor->engine.setSelectedNavMeshVertexPosition(position);
	}

	u32 getSelectedNavMeshCount()
	{
		return g_pEditor->engine.getSelectedNavMeshCount();
	}

	void setMeshInstanceAnimation(u64 instanceSid, u64 animSid)
	{
		g_pEditor->engine.setMeshInstanceAnimation(instanceSid, animSid);
	}

	u64 getMeshInstanceAnimation(u64 instanceSid)
	{
		return g_pEditor->engine.getMeshInstanceAnimation(instanceSid);
	}

	u32 getMeshInstanceCount()
	{
		return g_pEditor->engine.getMeshInstanceCount();
	}

	BSTR getMeshInstanceNameIndex(u32 i)
	{
		auto meshInstanceName = g_pEditor->engine.getMeshInstanceName(i);

		if (meshInstanceName)
		{
			return ANSItoBSTR(meshInstanceName);
		}
		else
		{
			return ::SysAllocString(L"meshInstance");
		}
	}

	BSTR getMeshInstanceName(u64 sid)
	{
		auto meshInstanceName = g_pEditor->engine.getMeshInstanceName(vx::StringID(sid));

		if (meshInstanceName)
		{
			return ANSItoBSTR(meshInstanceName);
		}
		else
		{
			return ::SysAllocString(L"meshInstance");
		}
	}

	u64 getMeshInstanceSidRaytrace(s32 mouseX, s32 mouseY)
	{
		return g_pEditor->engine.getMeshInstanceSid(mouseX, mouseY);
	}

	void setSelectedMeshInstance(u64 sid)
	{
		g_pEditor->engine.setSelectedMeshInstance(sid);
	}

	u64 getMeshInstanceSid(u32 i)
	{
		return g_pEditor->engine.getMeshInstanceSid(i);
	}

	u64 deselectMeshInstance()
	{
		return g_pEditor->engine.deselectMeshInstance();
	}

	BSTR getSelectedMeshInstanceName()
	{
		return ANSItoBSTR(g_pEditor->engine.getSelectedMeshInstanceName());
	}

	u64 getSelectedMeshInstanceSid()
	{
		return g_pEditor->engine.getSelectedMeshInstanceSid();
	}

	u64 getMeshInstanceMeshSid(u64 instanceSid)
	{
		return g_pEditor->engine.getMeshInstanceMeshSid(instanceSid);
	}

	void setMeshInstanceMeshSid(u64 instanceSid, u64 meshSid)
	{
		g_pEditor->engine.setMeshInstanceMeshSid(instanceSid, meshSid);
	}

	u64 getMeshInstanceMaterialSid(u64 instanceSid)
	{
		return g_pEditor->engine.getMeshInstanceMaterialSid(instanceSid);
	}

	void getMeshInstancePosition(u64 sid, vx::float3* position)
	{
		g_pEditor->engine.getMeshInstancePosition(sid, position);
	}

	void setMeshInstancePosition(u64 sid, const vx::float3 &translation)
	{
		g_pEditor->engine.setMeshInstancePosition(sid, translation);
	}

	void setMeshInstanceRotation(u64 sid, const vx::float3 &rotationDeg)
	{
		g_pEditor->engine.setMeshInstanceRotation(sid, rotationDeg);
	}

	void getMeshInstanceRotation(u64 sid, vx::float3* rotationDeg)
	{
		g_pEditor->engine.getMeshInstanceRotation(sid, rotationDeg);
	}

	void setMeshInstanceMaterial(u64 instanceSid, u64 materialSid)
	{
		g_pEditor->engine.setMeshInstanceMaterial(instanceSid, materialSid);
	}

	bool setMeshInstanceName(u64 instanceSid, const char* newName)
	{
		return g_pEditor->engine.setMeshInstanceName(instanceSid, newName);
	}

	u64 createMeshInstance()
	{
		return g_pEditor->engine.createMeshInstance().value;
	}

	void removeMeshInstance(u64 sid)
	{
		g_pEditor->engine.removeMeshInstance(sid);
	}

	void createLight()
	{
		g_pEditor->engine.createLight();
	}

	bool selectLight(s32 x, s32 y)
	{
		return g_pEditor->engine.selectLight(x, y);
	}

	void deselectLight()
	{
		g_pEditor->engine.deselectLight();
	}

	void getSelectLightPosition(vx::float3* position)
	{
		g_pEditor->engine.getSelectLightPosition(position);
	}

	void setSelectLightPosition(const vx::float3 &position)
	{
		g_pEditor->engine.setSelectLightPosition(position);
	}

	f32 getSelectLightFalloff()
	{
		return g_pEditor->engine.getSelectLightFalloff();
	}

	void setSelectLightLumen(f32 lumen)
	{
		g_pEditor->engine.setSelectLightLumen(lumen);
	}

	f32 getSelectLightLumen()
	{
		return g_pEditor->engine.getSelectLightLumen();
	}

	void setSelectLightFalloff(f32 falloff)
	{
		g_pEditor->engine.setSelectLightFalloff(falloff);
	}

	void showNavmesh(bool b)
	{
		g_pEditor->engine.showNavmesh(b);
	}

	void showInfluenceMap(bool b)
	{
		g_pEditor->engine.showInfluenceMap(b);
	}

	bool addWaypoint(s32 x, s32 y, vx::float3* position)
	{
		return g_pEditor->engine.addWaypoint(x, y, position);
	}

	void addWaypointPosition(const vx::float3 &position)
	{
		g_pEditor->engine.addWaypoint(position);
	}

	void removeWaypoint(const vx::float3 &position)
	{
		g_pEditor->engine.removeWaypoint(position);
	}

	void addSpawn()
	{
		g_pEditor->engine.addSpawn();
	}

	bool selectSpawn(s32 mouseX, s32 mouseY, u32* id)
	{
		return g_pEditor->engine.selectSpawn(mouseX, mouseY, id);
	}

	void getSpawnPosition(u32 id, vx::float3* position)
	{
		g_pEditor->engine.getSpawnPosition(id, position);
	}

	u32 getSpawnType(u32 id)
	{
		return g_pEditor->engine.getSpawnType(id);
	}

	void setSpawnPosition(u32 id, const vx::float3 &position)
	{
		g_pEditor->engine.setSpawnPosition(id, position);
	}

	void setSpawnType(u32 id, u32 type)
	{
		return g_pEditor->engine.setSpawnType(id, type);
	}

	u32 getMeshCount()
	{
		return g_pEditor->engine.getMeshCount();
	}

	BSTR getMeshName(u32 i)
	{
		auto meshName = g_pEditor->engine.getMeshName(i);

		if (meshName)
		{
			return ANSItoBSTR(meshName);
		}
		else
		{
			return ::SysAllocString(L"unknownMesh");
		}
	}

	u64 getMeshSid(u32 i)
	{
		return g_pEditor->engine.getMeshSid(i);
	}

	u32 getMaterialCount()
	{
		return g_pEditor->engine.getMaterialCount();
	}

	BSTR getMaterialNameIndex(u32 i)
	{
		auto name = g_pEditor->engine.getMaterialNameIndex(i);

		if (name)
		{
			return ANSItoBSTR(name);
		}
		else
		{
			return ::SysAllocString(L"unknownMaterial");
		}
	}

	BSTR getMaterialName(u64 i)
	{
		auto name = g_pEditor->engine.getMaterialName(i);

		if (name)
		{
			return ANSItoBSTR(name);
		}
		else
		{
			return ::SysAllocString(L"unknownMaterial");
		}
	}

	u64 getMaterialSid(u32 i)
	{
		return g_pEditor->engine.getMaterialSid(i);
	}

	u32 getAnimationCount()
	{
		return g_pEditor->engine.getAnimationCount();
	}

	BSTR getAnimationNameIndex(u32 i)
	{
		auto name = g_pEditor->engine.getAnimationNameIndex(i);

		if (name)
		{
			return ANSItoBSTR(name);
		}
		else
		{
			return ::SysAllocString(L"unknownAnimation");
		}
	}

	u64 getAnimationSidIndex(u32 i)
	{
		return g_pEditor->engine.getAnimationSidIndex(i);
	}

	u32 getMeshPhysxType(u64 sid)
	{
		return g_pEditor->engine.getMeshPhysxType(sid);
	}

	void setMeshPhysxType(u64 sid, u32 type)
	{
		g_pEditor->engine.setMeshPhysxType(sid, type);
	}

	u32 getMeshInstanceRigidBodyType(u64 sid)
	{
		return g_pEditor->engine.getMeshInstanceRigidBodyType(sid);
	}

	void setMeshInstanceRigidBodyType(u64 sid, u32 type)
	{
		g_pEditor->engine.setMeshInstanceRigidBodyType(sid, type);
	}

	u32 getJointCount()
	{
		return g_pEditor->engine.getJointCount();
	}

	void getJointData(u32 i, vx::float3* p0, vx::float3* q0, vx::float3* p1, vx::float3* q1, u64* sid0, u64* sid1, u32* limitEnabled, f32* limitMin, f32* limitMax)
	{
		g_pEditor->engine.getJointData(i, p0, q0, p1, q1, sid0, sid1, limitEnabled, limitMin, limitMax);
	}

	void addJoint(u64 sid)
	{
		g_pEditor->engine.addJoint(vx::StringID(sid));
	}

	void removeJoint(u32 index)
	{
		g_pEditor->engine.removeJoint(index);
	}

	bool selectJoint(s32 mouseX, s32 mouseY, u32* index)
	{
		return g_pEditor->engine.selectJoint(mouseX, mouseY, index);
	}

	void setJointPosition0(u32 index, const vx::float3 &p)
	{
		g_pEditor->engine.setJointPosition0(index, p);
	}

	void setJointPosition1(u32 index, const vx::float3 &p)
	{
		g_pEditor->engine.setJointPosition1(index, p);
	}

	void setJointBody0(u32 index, u64 sid)
	{
		g_pEditor->engine.setJointBody0(index, sid);
	}

	void setJointBody1(u32 index, u64 sid)
	{
		g_pEditor->engine.setJointBody1(index, sid);
	}

	void setJointRotation0(u32 index, const vx::float3 &p)
	{
		g_pEditor->engine.setJointRotation0(index, p);
	}

	void setJointRotation1(u32 index, const vx::float3 &p)
	{
		g_pEditor->engine.setJointRotation1(index, p);
	}

	void setJointLimit(u32 index, u32 enabled, f32 limitMin, f32 limitMax)
	{
		g_pEditor->engine.setJointLimit(index, enabled, limitMin, limitMax);
	}
}
#endif