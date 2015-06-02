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
#include "Light.h"
#include "Transform.h"
#include "EditorScene.h"

#ifdef _VX_EDITOR
namespace Editor
{
	struct Editor
	{
		Timer clock;
		Logfile logfile;
		EditorEngine engine;
		EditorScene scene;

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

	bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, u32 panelSizeX, u32 panelSizeY, u32 typeMesh, u32 typeMaterial, u32 typeScene)
	{
		EditorEngine::editor_setTypes(typeMesh, typeMaterial, typeScene);

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

	bool multiSelectNavMeshVertex(s32 mouseX, s32 mouseY)
	{
		return g_pEditor->engine.multiSelectNavMeshVertex(mouseX, mouseY);
	}

	void deselectNavMeshVertex()
	{
		g_pEditor->engine.deselectNavMeshVertex();
	}

	bool createNavMeshTriangleFromSelectedVertices()
	{
		return g_pEditor->engine.createNavMeshTriangleFromSelectedVertices();
	}

	bool addNavMeshVertex(s32 x, s32 y)
	{
		return g_pEditor->engine.addNavMeshVertex(x, y);
	}

	void deleteSelectedNavMeshVertex()
	{
		g_pEditor->engine.deleteSelectedNavMeshVertex();
	}

	void getSelectNavMeshVertexPosition(vx::float3* position)
	{
		*position = g_pEditor->engine.getSelectedNavMeshVertexPosition();
	}

	void setSelectNavMeshVertexPosition(const vx::float3 &position)
	{
		g_pEditor->engine.setSelectedNavMeshVertexPosition(position);
	}

	u32 getMeshInstanceCount()
	{
		return g_pEditor->engine.getMeshInstanceCount();
	}

	BSTR getMeshInstanceName(u32 i)
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

	bool selectMeshInstance(s32 x, s32 y)
	{
		return g_pEditor->engine.selectMeshInstance(x, y);
	}

	bool selectMeshInstanceIndex(u32 i)
	{
		return g_pEditor->engine.selectMeshInstance(i);
	}

	void deselectMeshInstance()
	{
		g_pEditor->engine.deselectMeshInstance();
	}

	BSTR getSelectedMeshInstanceName()
	{
		return ANSItoBSTR(g_pEditor->engine.getSelectedMeshInstanceName());
	}

	void updateSelectedMeshInstanceTransform(const vx::float3 &translation)
	{
		g_pEditor->engine.updateSelectedMeshInstanceTransform(translation);
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

	void addWaypoint(s32 x, s32 y)
	{
		g_pEditor->engine.addWaypoint(x, y);
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
			return ::SysAllocString(L"mesh");
		}
	}
}
#endif