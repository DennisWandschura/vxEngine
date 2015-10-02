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

class InfluenceMap;
class NavMeshGraph;
struct Waypoint;
struct Spawn;
class Material;
class NavMesh;
struct Joint;

template<typename T>
class Reference;

namespace vx
{
	class Camera;
	struct StringID;
}

namespace Graphics
{
	struct Light;
	struct LightGeometryProxy;
}

#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include <vxLib/Container/sorted_vector.h>

namespace Editor
{
	class MeshInstance;

	class RenderAspectInterface : public ::RenderAspectInterface
	{
	public:
		virtual ~RenderAspectInterface() {}

		virtual void addMeshInstance(const Editor::MeshInstance &instance)=0;
		virtual bool removeMeshInstance(const vx::StringID &sid) = 0;

		virtual bool setSelectedMeshInstance(const Editor::MeshInstance* instance) = 0;
		virtual void setSelectedMeshInstanceTransform(vx::Transform &transform) = 0;
		virtual bool setSelectedMeshInstanceMaterial(const Material* material) = 0;
		virtual bool setMeshInstanceMesh(const vx::StringID &sid, const vx::StringID &meshSid) = 0;

		virtual void moveCamera(f32 dirX, f32 dirY, f32 dirZ) = 0;
		virtual void VX_CALLCONV rotateCamera(const __m128 rotation) = 0;

		virtual void updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertex)[3], u8 selectedCount) = 0;
		virtual void updateNavMeshBuffer(const NavMesh &navMesh) = 0;
		virtual void updateInfluenceCellBuffer(const InfluenceMap &influenceMap) = 0;
		virtual void updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph) = 0;
		virtual void updateLightBuffer(const Graphics::Light* lights, u32 count) = 0;
		virtual void updateWaypoints(const Waypoint* w, u32 count) = 0;
		virtual void updateSpawns(const Spawn* spawns, u32 count) = 0;
		virtual void updateJoints(const Joint* joints, u32 count, const vx::sorted_vector<vx::StringID, Editor::MeshInstance> &meshinstances) = 0;
		virtual void updateLightGeometryProxies(const Graphics::LightGeometryProxy* proxies, u32 count) = 0;

		virtual void getViewMatrix(vx::mat4* viewMatrix) const = 0;
		virtual void getCameraPosition(vx::float4a* position) const = 0;

		virtual void showInfluenceMap(bool b, const InfluenceMap &influenceMap) = 0;
		virtual void showNavMesh(bool b, const NavMesh &navMesh, const NavMeshGraph &navMeshGraph) = 0;
	};
}

typedef Editor::RenderAspectInterface* (*CreateEditorRenderAspectFunction)();
typedef void(*DestroyEditorRenderAspectFunction)(Editor::RenderAspectInterface *p);