#include "SceneBase.h"
#include "Light.h"
#include "Spawn.h"
#include "Actor.h"

SceneBase::SceneBase()
	:m_pLights(),
	m_pSpawns(),
	m_actors()
{

}

SceneBase::SceneBase(SceneBase &&rhs)
	: m_pLights(std::move(rhs.m_pLights)),
	m_materials(std::move(rhs.m_materials)),
	m_meshes(std::move(rhs.m_meshes)),
	m_pSpawns(std::move(rhs.m_pSpawns)),
	m_actors(std::move(rhs.m_actors)),
	m_navMesh(std::move(rhs.m_navMesh)),
	m_lightCount(rhs.m_lightCount),
	m_vertexCount(rhs.m_vertexCount),
	m_indexCount(rhs.m_indexCount),
	m_spawnCount(rhs.m_spawnCount)
{
}

SceneBase::SceneBase(std::unique_ptr<Light[]> &&lights, U32 lightCount, vx::sorted_vector<vx::StringID64, Material*> &&materials,
	vx::sorted_vector<vx::StringID64, const vx::Mesh*> &&meshes, U32 vertexCount, U32 indexCount, std::unique_ptr<Spawn[]> &&spawns, U32 spawnCount,
	vx::sorted_vector<vx::StringID64, Actor>  &&actors, NavMesh &&navmesh)
	: m_pLights(std::move(lights)),
	m_materials(std::move(materials)),
	m_meshes(std::move(meshes)),
	m_pSpawns(std::move(spawns)),
	m_actors(std::move(actors)),
	m_navMesh(std::move(navmesh)),
	m_lightCount(lightCount),
	m_vertexCount(vertexCount),
	m_indexCount(indexCount),
	m_spawnCount(spawnCount)
{
}

SceneBase& SceneBase::operator = (SceneBase &&rhs)
{
	if (this != &rhs)
	{
		m_pLights = std::move(rhs.m_pLights);
		m_materials = std::move(rhs.m_materials);
		m_meshes = std::move(rhs.m_meshes);
		m_pSpawns = std::move(rhs.m_pSpawns);
		m_actors = std::move(rhs.m_actors);
		std::swap(m_navMesh, rhs.m_navMesh);
		m_lightCount = rhs.m_lightCount;
		m_vertexCount = rhs.m_vertexCount;
		m_indexCount = rhs.m_indexCount;
		m_spawnCount = rhs.m_spawnCount;
	}

	return *this;
}

SceneBase::~SceneBase()
{

}

const Light* SceneBase::getLights() const
{
	return m_pLights.get();
}

U32 SceneBase::getLightCount() const
{
	return m_lightCount;
}

const vx::sorted_vector<vx::StringID64, Material*>& SceneBase::getMaterials() const
{
	return m_materials;
}

U32 SceneBase::getMaterialCount() const
{
	return m_materials.size();
}

const vx::sorted_vector<vx::StringID64, const vx::Mesh*>& SceneBase::getMeshes() const
{
	return m_meshes;
}

U32 SceneBase::getVertexCount() const
{
	return m_vertexCount;
}

const Spawn* SceneBase::getSpawns() const
{
	return m_pSpawns.get();
}

U32 SceneBase::getSpawnCount() const
{
	return m_spawnCount;
}

const vx::sorted_vector<vx::StringID64, Actor>& SceneBase::getActors() const
{
	return m_actors;
}

const NavMesh& SceneBase::getNavMesh() const
{
	return m_navMesh;
}