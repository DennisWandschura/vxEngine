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
			engine(logfile),
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

	U32 addMeshInstance(U64 instanceSid, U64 meshSid, U64 materialSid, const vx::float3 &translation, const vx::float3 &rotation, const F32 scaling)
	{
		vx::Transform transform(translation, rotation, scaling);
		return g_pEditor->engine.editor_addMeshInstance(vx::make_sid(instanceSid), vx::make_sid(meshSid), vx::make_sid(materialSid), transform);
	}

	U32 getTransform(U64 instanceSid, vx::float3 &translation, vx::float3 &rotation, F32 &scaling)
	{
		return g_pEditor->engine.editor_getTransform(vx::make_sid(instanceSid), translation, rotation, scaling);
	}

	void updateTranslation(U64 instanceSid, const vx::float3 &translation)
	{
		g_pEditor->engine.editor_updateTranslation(vx::make_sid(instanceSid), translation);
	}

	U64 getSid(const char *str)
	{
		return vx::make_sid(str).m_value;
	}

	void saveScene(const char* name)
	{
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

	void raytraceMouse(I32 x, I32 y, U32 mode)
	{
		vx::float3 hitPos;
		auto result = g_pEditor->engine.raytraceMouse(x, y, &hitPos);

		if (result != 0 && mode == 1)
		{
			g_pEditor->engine.addWaypoint(hitPos);
		}
	}
}
#endif