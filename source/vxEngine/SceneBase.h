#pragma once

class Material;
class MeshInstance;

struct Light;
struct Spawn;
struct Actor;

namespace vx
{
	class Mesh;
	class StringID64;
}

#include <vxLib/Container/sorted_vector.h>
#include <memory>
#include "NavMesh.h"

class SceneBase
{
protected:
	std::unique_ptr<Light[]> m_pLights;
	vx::sorted_vector<vx::StringID64, Material*> m_materials{};
	vx::sorted_vector<vx::StringID64, const vx::Mesh*> m_meshes{};
	std::unique_ptr<Spawn[]> m_pSpawns;
	vx::sorted_vector<vx::StringID64, Actor> m_actors;
	NavMesh m_navMesh{};
	U32 m_lightCount{ 0 };
	U32 m_vertexCount{ 0 };
	U32 m_indexCount{ 0 };
	U32 m_spawnCount{ 0 };

	SceneBase();
	SceneBase(std::unique_ptr<Light[]> &&lights, U32 lightCount, vx::sorted_vector<vx::StringID64, Material*> &&materials,
		vx::sorted_vector<vx::StringID64, const vx::Mesh*> &&meshes, U32 vertexCount, U32 indexCount, std::unique_ptr<Spawn[]> &&spawns, U32 spawnCount,
		vx::sorted_vector<vx::StringID64, Actor>  &&actors, NavMesh &&navmesh);

public:
	SceneBase(SceneBase &&rhs);
	SceneBase(const SceneBase&) = delete;

	virtual ~SceneBase();

	SceneBase& operator = (const SceneBase&) = delete;
	SceneBase& operator = (SceneBase &&rhs);

	virtual void sortMeshInstances() = 0;

	virtual const MeshInstance* getMeshInstances() const = 0;
	virtual U32 getMeshInstanceCount() const = 0;

	const Light* getLights() const;
	U32 getLightCount() const;

	const vx::sorted_vector<vx::StringID64, Material*>& getMaterials() const;
	U32 getMaterialCount() const;

	const vx::sorted_vector<vx::StringID64, const vx::Mesh*>& getMeshes() const;
	U32 getVertexCount() const;

	const Spawn* getSpawns() const;
	U32 getSpawnCount() const;

	const vx::sorted_vector<vx::StringID64, Actor>& getActors() const;

	const NavMesh& getNavMesh() const;
};