/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "EditorScene.h"
#include "MeshInstance.h"
#include "Waypoint.h"
#include "SceneFile.h"
#include "Actor.h"
#include "Light.h"
#include "Spawn.h"

EditorSceneParams::~EditorSceneParams()
{

}

EditorScene::EditorScene()
	:m_meshInstances(), 
	m_waypoints(),
	m_meshInstanceNames()
{

}

EditorScene::EditorScene(EditorSceneParams &&params)
	: SceneBase(params.m_baseParams),
	m_meshInstances(std::move(params.m_meshInstances)),
	m_selectableLights(),
	m_materialNames(std::move(params.m_materialNames)),
	m_meshNames(std::move(params.m_meshNames)),
	m_actorNames(std::move(params.m_actorNames)),
	m_meshInstanceNames(std::move(params.m_meshInstanceNames))
{
	buildSelectableLights();

	m_selectableSpawns.reserve(m_spawnCount);
	for (u32 i = 0; i < m_spawnCount; ++i)
	{
		auto &spawn = m_pSpawns[i];

		AABB bounds;
		bounds.max = spawn.position + vx::float3(0.1f);
		bounds.min = spawn.position - vx::float3(0.1f);

		SelectableWrapper<Spawn> selected;
		selected.m_bounds = bounds;
		selected.m_ptr = &spawn;

		m_selectableSpawns.push_back(selected);
	}
}

EditorScene::~EditorScene()
{

}

EditorScene& EditorScene::operator = (EditorScene &&rhs)
{
	if (this != &rhs)
	{
		SceneBase::operator=(std::move(rhs));
		std::swap(m_meshInstances, rhs.m_meshInstances);
		std::swap(m_waypoints, rhs.m_waypoints);
		std::swap(m_selectableLights, rhs.m_selectableLights);
		std::swap(m_materialNames, rhs.m_materialNames);
		std::swap(m_meshNames, rhs.m_meshNames);
		std::swap(m_actorNames, rhs.m_actorNames);
		std::swap(m_meshInstanceNames, rhs.m_meshInstanceNames);
	}

	return *this;
}

void EditorScene::buildSelectableLights()
{
	m_selectableLights.clear();
	m_selectableLights.reserve(m_lightCount);
	for (u32 i = 0; i < m_lightCount; ++i)
	{
		auto &light = m_pLights[i];

		AABB bounds;
		bounds.max = light.m_position + vx::float3(0.1f);
		bounds.min = light.m_position - vx::float3(0.1f);

		SelectableWrapper<Light> selectLight;
		selectLight.m_bounds = bounds;
		selectLight.m_ptr = &light;

		m_selectableLights.push_back(selectLight);
	}
}

void EditorScene::sortMeshInstances()
{
	std::sort(m_meshInstances.begin(), m_meshInstances.end(), [&](const MeshInstance &lhs, const MeshInstance &rhs)
	{
		return (lhs.getMeshSid() < rhs.getMeshSid());
	});
}

Light* EditorScene::addLight(const Light &light)
{
#if _VX_EDITOR
	m_pLights.push_back(light);
	++m_lightCount;

	buildSelectableLights();

	return &m_pLights.back();
#else
	return nullptr;
#endif
}

u8 EditorScene::addMesh(vx::StringID sid, const char* name, const vx::MeshFile* pMesh)
{
	u8 result = 0;
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

u8 EditorScene::addMaterial(vx::StringID sid, const char* name, Material* pMaterial)
{
	u8 result = 0;
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

void EditorScene::addWaypoint(const Waypoint &wp)
{
	m_waypoints.push_back(wp);
}

const char* EditorScene::getMeshInstanceName(const vx::StringID &sid) const
{
	auto it = m_meshInstanceNames.find(sid);
	if (it == m_meshInstanceNames.end())
		return nullptr;

	return it->c_str();
}

const char* EditorScene::getMaterialName(const vx::StringID &sid) const
{
	auto it = m_materialNames.find(sid);
	if (it == m_materialNames.end())
		return nullptr;

	return it->c_str();
}

const char* EditorScene::getMeshName(const vx::StringID &sid) const
{
	auto it = m_meshNames.find(sid);
	if (it == m_meshNames.end())
		return nullptr;

	return it->c_str();
}

const char* EditorScene::getActorName(const vx::StringID &sid) const
{
	auto it = m_actorNames.find(sid);
	if (it == m_actorNames.end())
		return nullptr;

	return it->c_str();
}

const MeshInstance* EditorScene::getMeshInstances() const
{
	return m_meshInstances.data();
}

u32 EditorScene::getMeshInstanceCount() const
{
	return m_meshInstances.size();
}

Spawn* EditorScene::getSpawn(const Ray &ray)
{
	Spawn* result = nullptr;

	f32 a, b;
	for (auto &it : m_selectableSpawns)
	{
		if (it.m_bounds.intersects(ray, &a, &b))
		{
			result = it.m_ptr;
			break;
		}
	}

	return result;
}

Light* EditorScene::getLight(const Ray &ray)
{
	Light* result = nullptr;

	f32 a, b;
	for (auto &it : m_selectableLights)
	{
		if (it.m_bounds.intersects(ray, &a, &b))
		{
			result = it.m_ptr;
			break;
		}
	}

	return result;
}

void EditorScene::updateLightPositions()
{
	for (u32 i = 0; i < m_lightCount; ++i)
	{
		auto &light = m_pLights[i];

		AABB bounds;
		bounds.max = light.m_position + vx::float3(0.1f);
		bounds.min = light.m_position - vx::float3(0.1f);

		m_selectableLights[i].m_bounds = bounds;
	}
}