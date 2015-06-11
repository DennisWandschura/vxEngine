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
#include <comutil.h>

#ifdef _VX_EDITOR
#define DLL_EXPORT __declspec( dllexport )

namespace Editor
{
	extern "C" DLL_EXPORT bool initializeEditor(intptr_t hwndPanel, intptr_t hwndTmp, u32 panelSizeX, u32 panelSizeY, u32 typeMesh, u32 typeMaterial, u32 typeScene, u32 typeFbx);
	extern "C" DLL_EXPORT void shutdownEditor();

	extern "C" DLL_EXPORT void frame();

	extern "C" DLL_EXPORT void loadFile(const char *filename, u32 type, LoadFileCallback f);

	extern "C" DLL_EXPORT u64 getSid(const char *str);

	extern "C" DLL_EXPORT void saveScene(const char* name);

	extern "C" DLL_EXPORT void moveCamera(f32 dirX, f32 dirY, f32 dirZ);

	extern "C" DLL_EXPORT void rotateCamera(f32 dirX, f32 dirY, f32 dirZ);

	extern "C" DLL_EXPORT bool addNavMeshVertex(s32 x, s32 y, vx::float3* position);
	extern "C" DLL_EXPORT void removeNavMeshVertex(const vx::float3 &position);
	extern "C" DLL_EXPORT void removeSelectedNavMeshVertex();
	extern "C" DLL_EXPORT bool selectNavMeshVertex(s32 x, s32 y);
	extern "C" DLL_EXPORT bool selectNavMeshVertexIndex(u32 index);
	extern "C" DLL_EXPORT bool selectNavMeshVertexPosition(const vx::float3 &position);
	extern "C" DLL_EXPORT bool multiSelectNavMeshVertex(s32 mouseX, s32 mouseY);
	extern "C" DLL_EXPORT u32 deselectNavMeshVertex();
	extern "C" DLL_EXPORT bool createNavMeshTriangleFromSelectedVertices(vx::uint3* selected);
	extern "C" DLL_EXPORT void createNavMeshTriangleFromIndices(const vx::uint3 &indices);
	extern "C" DLL_EXPORT void removeNavMeshTriangle();
	extern "C" DLL_EXPORT bool getSelectNavMeshVertexPosition(vx::float3* position);
	extern "C" DLL_EXPORT void setSelectNavMeshVertexPosition(const vx::float3 &position);
	extern "C" DLL_EXPORT u32 getSelectedNavMeshCount();

	extern "C" DLL_EXPORT u32 getMeshInstanceCount();
	extern "C" DLL_EXPORT BSTR getMeshInstanceNameIndex(u32 i);
	extern "C" DLL_EXPORT BSTR getMeshInstanceName(u64 sid);
	extern "C" DLL_EXPORT bool selectMeshInstance(s32 x, s32 y);
	extern "C" DLL_EXPORT bool selectMeshInstanceIndex(u32 i);
	extern "C" DLL_EXPORT bool selectMeshInstanceSid(u64 sid);
	extern "C" DLL_EXPORT u64 getMeshInstanceSid(u32 i);
	extern "C" DLL_EXPORT u64 deselectMeshInstance();
	extern "C" DLL_EXPORT BSTR getSelectedMeshInstanceName();
	extern "C" DLL_EXPORT u64 getSelectedMeshInstanceSid();

	extern "C" DLL_EXPORT u64 getMeshInstanceMeshSid(u64 instanceSid);
	extern "C" DLL_EXPORT void setMeshInstanceMeshSid(u64 instanceSid, u64 meshSid);

	extern "C" DLL_EXPORT void getMeshInstancePosition(u64 sid, vx::float3* position);
	extern "C" DLL_EXPORT void setMeshInstancePosition(u64 sid, const vx::float3 &translation);

	extern "C" DLL_EXPORT void setMeshInstanceRotation(u64 sid, const vx::float3 &rotationDeg);
	extern "C" DLL_EXPORT void getMeshInstanceRotation(u64 sid, vx::float3* rotationDeg);

	extern "C" DLL_EXPORT u64 getMeshInstanceMaterialSid(u64 instanceSid);
	extern "C" DLL_EXPORT void setMeshInstanceMaterial(u64 instanceSid, u64 materialSid);

	extern "C" DLL_EXPORT u64 createMeshInstance();
	extern "C" DLL_EXPORT void removeMeshInstance(u64 sid);

	extern "C" DLL_EXPORT bool setMeshInstanceName(u64 instanceSid, const char* newName);

	extern "C" DLL_EXPORT void createLight();
	extern "C" DLL_EXPORT bool selectLight(s32 x, s32 y);
	extern "C" DLL_EXPORT void deselectLight();
	extern "C" DLL_EXPORT void getSelectLightPosition(vx::float3* position);
	extern "C" DLL_EXPORT void setSelectLightPosition(const vx::float3 &position);

	extern "C" DLL_EXPORT void showNavmesh(bool b);
	extern "C" DLL_EXPORT void showInfluenceMap(bool b);

	extern "C" DLL_EXPORT bool addWaypoint(s32 x, s32 y, vx::float3* position);
	extern "C" DLL_EXPORT void addWaypointPosition(const vx::float3 &position);
	extern "C" DLL_EXPORT void removeWaypoint(const vx::float3 &position);

	extern "C" DLL_EXPORT u32 getMeshCount();
	extern "C" DLL_EXPORT BSTR getMeshName(u32 i);
	extern "C" DLL_EXPORT u64 getMeshSid(u32 i);

	extern "C" DLL_EXPORT u32 getMaterialCount();
	extern "C" DLL_EXPORT BSTR getMaterialName(u32 i);
	extern "C" DLL_EXPORT u64 getMaterialSid(u32 i);
}
#endif