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

	extern "C" DLL_EXPORT void showNavmesh(bool b);
	extern "C" DLL_EXPORT void showInfluenceMap(bool b);
}
#endif