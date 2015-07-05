#pragma once

class InfluenceMap;
class NavMeshGraph;
struct Light;
struct Waypoint;
struct Spawn;
class Material;
class NavMesh;

template<typename T>
class Reference;

namespace vx
{
	class Camera;
	struct StringID;
}

#include <vxEngineLib/RenderAspectInterface.h>

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
		virtual bool setSelectedMeshInstanceMaterial(const Reference<Material> &material) const = 0;
		virtual bool setMeshInstanceMesh(const vx::StringID &sid, const vx::StringID &meshSid) = 0;

		virtual void moveCamera(f32 dirX, f32 dirY, f32 dirZ) = 0;
		virtual void VX_CALLCONV rotateCamera(const __m128 rotation) = 0;

		virtual void updateNavMeshBuffer(const NavMesh &navMesh, u32(&selectedVertex)[3], u8 selectedCount) = 0;
		virtual void updateNavMeshBuffer(const NavMesh &navMesh) = 0;
		virtual void updateInfluenceCellBuffer(const InfluenceMap &influenceMap) = 0;
		virtual void updateNavMeshGraphNodesBuffer(const NavMeshGraph &navMeshGraph) = 0;
		virtual void updateLightBuffer(const Light* lights, u32 count) = 0;
		virtual void updateWaypoints(const Waypoint* w, u32 count) = 0;
		virtual void updateSpawns(const Spawn* spawns, u32 count) = 0;

		virtual const vx::Camera& getCamera() const = 0;
	};
}

typedef Editor::RenderAspectInterface* (*CreateEditorRenderAspectFunction)(const RenderAspectDescription &desc, RenderAspectInitializeError* error);
typedef void(*DestroyEditorRenderAspectFunction)(Editor::RenderAspectInterface *p);