#pragma once

class MeshInstance;
struct Waypoint;

#include "SceneBase.h"

class Scene : public SceneBase
{
	std::unique_ptr<MeshInstance[]> m_pMeshInstances;
	std::unique_ptr<Waypoint[]> m_waypoints;
	AABB m_bounds;
	U32 m_meshInstanceCount{ 0 };
	U32 m_waypointCount{ 0 };

public:
	Scene();
	Scene(std::unique_ptr<MeshInstance[]> &&meshInstances, U32 numMeshInstances, std::unique_ptr<Light[]> &&lights, U32 numLights,
		vx::sorted_vector<vx::StringID64, Material*> &&materials, vx::sorted_vector<vx::StringID64, const vx::Mesh*> &&meshes, U32 vertexCount, U32 indexCount, 
		std::unique_ptr<Spawn[]> &&spawns, U32 spawnCount, vx::sorted_vector<vx::StringID64, Actor> &&actors, NavMesh &&navmesh, const AABB &bounds);

	Scene(const Scene&) = delete;
	Scene(Scene &&rhs);
	~Scene();

	Scene& operator=(const Scene&) = delete;
	Scene& operator=(Scene &&rhs);

	// sorts by material type, then by mesh
	void sortMeshInstances() override;

	const MeshInstance* getMeshInstances() const override;
	U32 getMeshInstanceCount() const override;
};