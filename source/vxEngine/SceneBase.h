#pragma once

class Material;
class MeshInstance;

struct Light;
struct Spawn;
struct Actor;
class SceneFile;

namespace vx
{
	class Mesh;
}

#include <vxLib/Container/sorted_vector.h>
#include <memory>
#include "NavMesh.h"
#include <vxLib/StringID.h>

struct SceneBaseParams
{
	std::unique_ptr<Light[]> m_pLights;
	vx::sorted_vector<vx::StringID, Material*> m_materials;
	vx::sorted_vector<vx::StringID, const vx::Mesh*> m_meshes;
	std::unique_ptr<Spawn[]> m_pSpawns;
	vx::sorted_vector<vx::StringID, Actor> m_actors;
	NavMesh m_navMesh;
	U32 m_lightCount;
	U32 m_vertexCount;
	U32 m_indexCount;
	U32 m_spawnCount;

	~SceneBaseParams();
};

class SceneBase
{
protected:
	std::unique_ptr<Light[]> m_pLights;
	vx::sorted_vector<vx::StringID, Material*> m_materials{};
	vx::sorted_vector<vx::StringID, const vx::Mesh*> m_meshes{};
	std::unique_ptr<Spawn[]> m_pSpawns;
	vx::sorted_vector<vx::StringID, Actor> m_actors;
	NavMesh m_navMesh{};
	U32 m_lightCount{ 0 };
	U32 m_vertexCount{ 0 };
	U32 m_indexCount{ 0 };
	U32 m_spawnCount{ 0 };

	SceneBase();
	SceneBase(SceneBaseParams &params);

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

	const vx::sorted_vector<vx::StringID, Material*>& getMaterials() const;
	U32 getMaterialCount() const;

	const vx::sorted_vector<vx::StringID, const vx::Mesh*>& getMeshes() const;
	U32 getVertexCount() const;

	const Spawn* getSpawns() const;
	U32 getSpawnCount() const;

	const vx::sorted_vector<vx::StringID, Actor>& getActors() const;

	NavMesh& getNavMesh();
	const NavMesh& getNavMesh() const;
};