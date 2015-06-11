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

#ifdef _VX_EDITOR
namespace Editor
{
	struct Editor
	{
		Timer clock;
		Logfile logfile;
		EditorEngine engine;
		Editor::Scene scene;

		Editor()
			:clock(),
			logfile(clock),
			engine(),
			scene()
		{
		}
	};

	Editor* g_pEditor{ nullptr };
	void* g_pMemory{ nullptr };
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

	bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, u32 panelSizeX, u32 panelSizeY, u32 typeMesh, u32 typeMaterial, u32 typeScene, u32 typeFbx)
	{
		EditorEngine::editor_setTypes(typeMesh, typeMaterial, typeScene, typeFbx);

		g_pMemory = ::operator new(sizeof(Editor));
		if (!g_pMemory)
			return false;

		auto pEditor = new(g_pMemory)Editor();

		pEditor->logfile.create("editor_log.xml");
		if (!pEditor->engine.initializeEditor((HWND)hwndPanel, (HWND)hwndTmp, vx::uint2(panelSizeX, panelSizeY), &pEditor->scene))
			return false;

		g_pEditor = pEditor;

		g_pEditor->engine.editor_start();

		auto frequency = Timer::getFrequency();
		g_invFrequency = 1.0 / frequency;
		QueryPerformanceCounter(&g_last);

		return true;
	}

	void shutdownEditor()
	{
		if (g_pEditor)
		{
			puts("Shutting down...");
			g_pEditor->engine.stop();
			g_pEditor->engine.shutdownEditor();
			g_pEditor->logfile.close();
			destroy(g_pEditor);
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

		g_pEditor->engine.editor_render();

	}

	void loadFile(const char *filename, unsigned int type, LoadFileCallback f)
	{
		g_pEditor->engine.editor_loadFile(filename, type, f);
	}

	u64 getSid(const char *str)
	{
		return vx::make_sid(str).value;
	}

	void saveScene(const char* name)
	{
		printf("%s\n", name);
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
		auto meshInstanceName = g_pEditor->engine.getMeshInstanceName(sid);

		if (meshInstanceName)
		{
			return ANSItoBSTR(meshInstanceName);
		}
		else
		{
			return ::SysAllocString(L"meshInstance");
		}
	}

	u64 getMeshInstanceSid(u32 i)
	{
		return g_pEditor->engine.getMeshInstanceSid(i);
	}

	bool selectMeshInstance(s32 x, s32 y)
	{
		return g_pEditor->engine.selectMeshInstance(x, y);
	}

	bool selectMeshInstanceIndex(u32 i)
	{
		return g_pEditor->engine.selectMeshInstance(i);
	}

	bool selectMeshInstanceSid(u64 sid)
	{
		return g_pEditor->engine.selectMeshInstance(sid);
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
}
#endif