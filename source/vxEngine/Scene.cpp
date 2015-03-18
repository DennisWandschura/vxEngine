#include "Scene.h"
#include <algorithm>
#include "MeshInstance.h"
#include "Waypoint.h"

Scene::Scene()
	:m_bounds()
{
}

Scene::Scene(std::unique_ptr<MeshInstance[]> &&meshInstances, U32 numMeshInstances, std::unique_ptr<Light[]> &&lights, U32 lightCount,
	vx::sorted_vector<vx::StringID64, Material*> &&materials, vx::sorted_vector<vx::StringID64, const vx::Mesh*> &&meshes, U32 vertexCount, U32 indexCount,
	std::unique_ptr<Spawn[]> &&spawns, U32 spawnCount, vx::sorted_vector<vx::StringID64, Actor>  &&actors, NavMesh &&navmesh, const AABB &bounds)
	: SceneBase(std::move(lights), lightCount, std::move(materials), std::move(meshes), vertexCount, indexCount, std::move(spawns), spawnCount, std::move(actors), std::move(navmesh)),
	m_pMeshInstances(std::move(meshInstances)),
	m_bounds(bounds),
	m_meshInstanceCount(numMeshInstances)
{
}

Scene::Scene(Scene &&rhs)
	:SceneBase(std::move(rhs)),
	m_pMeshInstances(std::move(rhs.m_pMeshInstances)),
	m_meshInstanceCount(rhs.m_meshInstanceCount)
{
}

Scene::~Scene()
{

}

Scene& Scene::operator = (Scene &&rhs)
{
	SceneBase::operator=(std::move(rhs));
	if (this != &rhs)
	{
		m_pMeshInstances = std::move(rhs.m_pMeshInstances);
		m_meshInstanceCount = rhs.m_meshInstanceCount;
	}

	return *this;
}

void Scene::sortMeshInstances()
{
	std::sort(m_pMeshInstances.get(), m_pMeshInstances.get() + m_meshInstanceCount, [&](const MeshInstance &lhs, const MeshInstance &rhs)
	{
		return (lhs.getMeshSid() < rhs.getMeshSid());
	});
}

const MeshInstance* Scene::getMeshInstances() const 
{
	return m_pMeshInstances.get(); 
}

U32 Scene::getMeshInstanceCount() const
{ 
	return m_meshInstanceCount; 
}