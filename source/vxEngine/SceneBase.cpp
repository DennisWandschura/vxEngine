#include "SceneBase.h"
#include "Light.h"
#include "Spawn.h"
#include "Actor.h"

SceneBaseParams::~SceneBaseParams()
{

}

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

SceneBase::SceneBase(SceneBaseParams &params)
	: m_pLights(std::move(params.m_pLights)),
	m_materials(std::move(params.m_materials)),
	m_meshes(std::move(params.m_meshes)),
	m_pSpawns(std::move(params.m_pSpawns)),
	m_actors(std::move(params.m_actors)),
	m_navMesh(std::move(params.m_navMesh)),
	m_lightCount(params.m_lightCount),
	m_vertexCount(params.m_vertexCount),
	m_indexCount(params.m_indexCount),
	m_spawnCount(params.m_spawnCount)
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

const vx::sorted_vector<vx::StringID, Material*>& SceneBase::getMaterials() const
{
	return m_materials;
}

U32 SceneBase::getMaterialCount() const
{
	return m_materials.size();
}

const vx::sorted_vector<vx::StringID, const vx::Mesh*>& SceneBase::getMeshes() const
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

const vx::sorted_vector<vx::StringID, Actor>& SceneBase::getActors() const
{
	return m_actors;
}

NavMesh& SceneBase::getNavMesh()
{
	return m_navMesh;
}

const NavMesh& SceneBase::getNavMesh() const
{
	return m_navMesh;
}