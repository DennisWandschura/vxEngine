#include "Scene.h"
#include <algorithm>
#include "MeshInstance.h"
#include "Waypoint.h"

SceneParams::~SceneParams()
{

}

Scene::Scene()
{
}

Scene::Scene(SceneParams &params)
	: SceneBase(params.m_baseParams),
	m_pMeshInstances(std::move(params.m_pMeshInstances)),
	m_waypoints(std::move(params.m_waypoints)),
	m_meshInstanceCount(params.m_meshInstanceCount),
	m_waypointCount(params.m_waypointCount)
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