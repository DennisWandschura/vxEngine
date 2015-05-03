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
		Clock clock;
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
	void* g_pMemory{nullptr};
	LARGE_INTEGER g_last{};
	F64 g_invFrequency{1.0};
	const U32 g_hz = 40u;
	const F32 g_dt = 1.0f / g_hz;

	template<class T>
	void destroy(T *p)
	{
		p->~T();
	}

	bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, U32 panelSizeX, U32 panelSizeY, U32 typeMesh, U32 typeMaterial, U32 typeScene)
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

		auto frequency = Clock::getFrequency();
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
		F32 frameTime = frameTicks * g_invFrequency * 0.001f;
		frameTime = fminf(frameTime, g_dt);

		g_last = current;

		g_pEditor->engine.editor_render(frameTime);

	}

	void loadFile(const char *filename, unsigned int type, LoadFileCallback f)
	{
		g_pEditor->engine.editor_loadFile(filename, type, f);
	}

	U64 getSid(const char *str)
	{
		return vx::make_sid(str);
	}

	void saveScene(const char* name)
	{
		printf("%s\n",name);
		g_pEditor->engine.editor_saveScene(name);
	}

	void moveCamera(F32 dirX, F32 dirY, F32 dirZ)
	{
		g_pEditor->engine.editor_moveCamera(dirX, dirY, dirZ);
	}

	void rotateCamera(F32 dirX, F32 dirY, F32 dirZ)
	{
		g_pEditor->engine.editor_rotateCamera(dirX, dirY, dirZ);
	}

	bool selectNavMeshVertex(I32 x, I32 y)
	{
		return g_pEditor->engine.selectNavMeshVertex(x, y);
	}

	bool multiSelectNavMeshVertex(I32 mouseX, I32 mouseY)
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

	bool addNavMeshVertex(I32 x, I32 y)
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

	bool selectMesh(I32 x, I32 y)
	{
		return g_pEditor->engine.selectMesh(x, y);
	}

	void deselectMesh()
	{
		g_pEditor->engine.deselectMesh();
	}

	void updateSelectedMeshInstanceTransform(const vx::float3 &translation)
	{
		g_pEditor->engine.updateSelectedMeshInstanceTransform(translation);
	}

	bool selectLight(I32 x, I32 y)
	{
		return g_pEditor->engine.selectLight(x, y);
	}

	void showNavmesh(bool b)
	{
		g_pEditor->engine.showNavmesh(b);
	}

	void showInfluenceMap(bool b)
	{
		g_pEditor->engine.showInfluenceMap(b);
	}
}
#endif