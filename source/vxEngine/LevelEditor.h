#pragma once

#include <vxLib/math/Vector.h>
#include "Editor.h"

#ifdef _VX_EDITOR
#define DLL_EXPORT __declspec( dllexport )

namespace Editor
{
	extern "C" DLL_EXPORT bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, U32 panelSizeX, U32 panelSizeY, U32 typeMesh, U32 typeMaterial, U32 typeScene);
	extern "C" DLL_EXPORT void shutdownEditor();

	extern "C" DLL_EXPORT void frame();

	extern "C" DLL_EXPORT void loadFile(const char *filename, U32 type, LoadFileCallback f);

	extern "C" DLL_EXPORT U64 getSid(const char *str);

	extern "C" DLL_EXPORT void saveScene(const char* name);

	extern "C" DLL_EXPORT void moveCamera(F32 dirX, F32 dirY, F32 dirZ);

	extern "C" DLL_EXPORT void rotateCamera(F32 dirX, F32 dirY, F32 dirZ);

	extern "C" DLL_EXPORT bool addNavMeshVertex(I32 x, I32 y);
	extern "C" DLL_EXPORT void deleteSelectedNavMeshVertex();
	extern "C" DLL_EXPORT bool selectNavMeshVertex(I32 x, I32 y);
	extern "C" DLL_EXPORT bool multiSelectNavMeshVertex(I32 mouseX, I32 mouseY);
	extern "C" DLL_EXPORT void deselectNavMeshVertex();
	extern "C" DLL_EXPORT bool createNavMeshTriangleFromSelectedVertices();
	extern "C" DLL_EXPORT void getSelectNavMeshVertexPosition(vx::float3* position);
	extern "C" DLL_EXPORT void setSelectNavMeshVertexPosition(const vx::float3 &position);

	extern "C" DLL_EXPORT bool selectMesh(I32 x, I32 y);
	extern "C" DLL_EXPORT void deselectMesh();
	extern "C" DLL_EXPORT void updateSelectedMeshInstanceTransform(const vx::float3 &translation);

	extern "C" DLL_EXPORT bool selectLight(I32 x, I32 y);
}
#endif