#pragma once

#include <vxLib/math/Vector.h>
#include <Windows.h>

#ifdef _VX_EDITOR
#define DLL_EXPORT __declspec( dllexport )

namespace Editor
{
	extern "C" DLL_EXPORT bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, U32 panelSizeX, U32 panelSizeY, U32 typeMesh, U32 typeMaterial, U32 typeScene);
	extern "C" DLL_EXPORT void shutdownEditor();

	extern "C" DLL_EXPORT void frame();

	extern "C" DLL_EXPORT void loadFile(const char *filename, U32 type, LoadFileCallback f);
	// returns 1 on success, 0 on failure
	extern "C" DLL_EXPORT U32 addMeshInstance(U64 instanceSid, U64 meshSid, U64 materialSid, const vx::float3 &translation, const vx::float3 &rotation, const F32 scaling);

	extern "C" DLL_EXPORT U32 getTransform(U64 instanceSid, vx::float3 &translation, vx::float3 &rotation, F32 &scaling);
	extern "C" DLL_EXPORT void updateTranslation(U64 instanceSid, const vx::float3 &translation);

	extern "C" DLL_EXPORT U64 getSid(const char *str);

	extern "C" DLL_EXPORT void saveScene(const char* name);

	extern "C" DLL_EXPORT void moveCamera(F32 dirX, F32 dirY, F32 dirZ);

	extern "C" DLL_EXPORT void rotateCamera(F32 dirX, F32 dirY, F32 dirZ);

	extern "C" DLL_EXPORT void raytraceMouse(I32 x, I32 y, U32 mode);
}
#endif