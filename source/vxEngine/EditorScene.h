#pragma once

class MeshInstance;
struct Waypoint;

namespace vx
{
	struct Transform;
}

#include "SceneBase.h"
#include <vector>

class EditorScene : public SceneBase
{
	std::vector<MeshInstance> m_meshInstances;
	std::vector<Waypoint> m_waypoints;

	vx::sorted_vector<vx::StringID64, MeshInstance> m_sortedMeshInstances;
	vx::sorted_vector<vx::StringID64, char[32]> m_materialNames{};
	vx::sorted_vector<vx::StringID64, char[32]> m_meshNames{};
	vx::sorted_vector<vx::StringID64, char[32]> m_actorNames{};

public:
	EditorScene();
	EditorScene(EditorScene &&rhs);
	EditorScene(std::vector<MeshInstance> &&meshInstances, std::unique_ptr<Light[]> &&lights, U32 lightCount, vx::sorted_vector<vx::StringID64, Material*> &&materials,
		vx::sorted_vector<vx::StringID64, const vx::Mesh*> &&meshes, U32 vertexCount, U32 indexCount, 
		std::unique_ptr<Spawn[]> &&spawns, U32 spawnCount, vx::sorted_vector<vx::StringID64, Actor>  &&actors, NavMesh &&navmesh);
	~EditorScene();

	EditorScene& operator = (EditorScene &&rhs);

	void sortMeshInstances() override;

	// returns 1 on insert, 0 if already present
	U8 addMesh(const vx::StringID64 &sid, const char* name, const vx::Mesh* pMesh);
	// returns 1 on insert, 0 if already present
	U8 addMaterial(const vx::StringID64 &sid, const char* name, Material* pMaterial);
	// returns 1 on insert, 0 if mesh or material is missing
	U8 addMeshInstance(const vx::StringID64 &instanceSid, const vx::StringID64 &meshSid, const vx::StringID64 &materialSid, const vx::Transform &transform);
	void addWaypoint(const Waypoint &wp);

	MeshInstance* findMeshInstance(const vx::StringID64 &instanceSid);

	const vx::sorted_vector<vx::StringID64, MeshInstance>& getMeshInstancesSortedByName() const { return m_sortedMeshInstances; }
	const MeshInstance* getMeshInstances() const override;
	U32 getMeshInstanceCount() const override;

	const char* getMaterialName(const vx::StringID64 &sid) const;
	const char* getMeshName(const vx::StringID64 &sid) const;
	const char* getActorName(const vx::StringID64 &sid) const;
};