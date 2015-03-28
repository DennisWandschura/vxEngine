#include "EditorScene.h"
#include "MeshInstance.h"
#include "Waypoint.h"

EditorSceneParams::~EditorSceneParams()
{

}

EditorScene::EditorScene()
	:m_meshInstances(), 
	m_waypoints(),
	m_sortedMeshInstances()
{

}

EditorScene::EditorScene(EditorSceneParams &params)
	: SceneBase(params.m_baseParams),
	m_meshInstances(std::move(params.m_meshInstances))
{
}

EditorScene::~EditorScene()
{

}

EditorScene& EditorScene::operator = (EditorScene &&rhs)
{
	if (this != &rhs)
	{
		assert(false);
		m_sortedMeshInstances = std::move(m_sortedMeshInstances);
	}

	return *this;
}

void EditorScene::sortMeshInstances()
{
	std::sort(m_meshInstances.begin(), m_meshInstances.end(), [&](const MeshInstance &lhs, const MeshInstance &rhs)
	{
		return (lhs.getMeshSid() < rhs.getMeshSid());
	});
}

U8 EditorScene::addMesh(const vx::StringID64 &sid, const char* name, const vx::Mesh* pMesh)
{
	U8 result = 0;
	auto it = m_meshes.find(sid);
	if (it == m_meshes.end())
	{
		char buffer[32];
		strcpy_s(buffer, name);

		m_meshNames.insert(sid, buffer);

		m_meshes.insert(sid, pMesh);
		result = 1;
	}

	return result;
}

U8 EditorScene::addMaterial(const vx::StringID64 &sid, const char* name, Material* pMaterial)
{
	U8 result = 0;
	auto it = m_materials.find(sid);
	if (it == m_materials.end())
	{
		char buffer[32];
		strcpy_s(buffer, name);
		m_materialNames.insert(sid, buffer);
		m_materials.insert(sid, pMaterial);
		result = 1;
	}

	return result;
}

U8 EditorScene::addMeshInstance(const vx::StringID64 &instanceSid, const vx::StringID64 &meshSid, const vx::StringID64 &materialSid, const vx::Transform &transform)
{
	auto itMesh = m_meshes.find(meshSid);
	auto itMaterial = m_materials.find(materialSid);

	if (itMesh == m_meshes.end() ||
		itMaterial == m_materials.end())
		return 0;

	MeshInstance instance(meshSid, materialSid, transform);
	m_sortedMeshInstances.insert(instanceSid, std::move(instance));

	m_meshInstances.push_back(instance);

	return 1;
}

void EditorScene::addWaypoint(const Waypoint &wp)
{
	m_waypoints.push_back(wp);
}

MeshInstance* EditorScene::findMeshInstance(const vx::StringID64 &instanceSid)
{
	auto it = m_sortedMeshInstances.find(instanceSid);
	if (it == m_sortedMeshInstances.end())
		return nullptr;

	return &(*it);
}

const char* EditorScene::getMaterialName(const vx::StringID64 &sid) const
{
	auto it = m_materialNames.find(sid);
	if (it == m_materialNames.end())
		return nullptr;

	return (*it);
}

const char* EditorScene::getMeshName(const vx::StringID64 &sid) const
{
	auto it = m_meshNames.find(sid);
	if (it == m_meshNames.end())
		return nullptr;

	return (*it);
}

const char* EditorScene::getActorName(const vx::StringID64 &sid) const
{
	auto it = m_actorNames.find(sid);
	if (it == m_actorNames.end())
		return nullptr;

	return (*it);
}

const MeshInstance* EditorScene::getMeshInstances() const
{
	return m_meshInstances.data();
}

U32 EditorScene::getMeshInstanceCount() const
{
	return m_meshInstances.size();
}