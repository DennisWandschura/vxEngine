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
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/copy.h>

namespace Editor
{
	SceneParams::~SceneParams()
	{

	}

	Scene::Scene()
		:m_meshInstances()
	{

	}

	Scene::Scene(SceneParams &&params)
		: SceneBase(params.m_baseParams),
		m_meshInstances(std::move(params.m_meshInstances)),
		m_selectableLights(),
		m_materialNames(std::move(params.m_materialNames)),
		m_meshNames(std::move(params.m_meshNames)),
		m_actorNames(std::move(params.m_actorNames))
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

	Scene::~Scene()
	{

	}

	Scene& Scene::operator = (Scene &&rhs)
	{
		if (this != &rhs)
		{
			SceneBase::operator=(std::move(rhs));
			std::swap(m_meshInstances, rhs.m_meshInstances);
			std::swap(m_selectableLights, rhs.m_selectableLights);
			std::swap(m_selectableSpawns, rhs.m_selectableSpawns);
			std::swap(m_selectableWaypoints, rhs.m_selectableWaypoints);
			std::swap(m_materialNames, rhs.m_materialNames);
			std::swap(m_meshNames, rhs.m_meshNames);
			std::swap(m_actorNames, rhs.m_actorNames);
		}

		return *this;
	}

	void Scene::buildSelectableLights()
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

	void Scene::buildSelectableWaypoints()
	{
		m_selectableWaypoints.clear();
		m_selectableWaypoints.reserve(m_waypointCount);
		for (u32 i = 0; i < m_waypointCount; ++i)
		{
			auto &waypoint = m_waypoints[i];

			AABB bounds;
			bounds.max = waypoint.position + vx::float3(0.1f);
			bounds.min = waypoint.position - vx::float3(0.1f);

			SelectableWrapper<Waypoint> selectLight;
			selectLight.m_bounds = bounds;
			selectLight.m_ptr = &waypoint;

			m_selectableWaypoints.push_back(selectLight);
		}
	}

	void Scene::copy(Scene* dst) const
	{
		SceneBase::copy(dst);

		copySortedVector(&dst->m_meshInstances, m_meshInstances);
		dst->m_selectableLights = m_selectableLights;
		dst->m_selectableSpawns = m_selectableSpawns;
		dst->m_selectableWaypoints = m_selectableWaypoints;
		copySortedVector(&dst->m_materialNames, m_materialNames);
		copySortedVector(&dst->m_meshNames, m_meshNames);
		copySortedVector(&dst->m_actorNames, m_actorNames);
	}

	void Scene::sortMeshInstances()
	{
	}

	void Scene::removeUnusedMaterials()
	{
		vx::sorted_vector<vx::StringID, Material*> newMaterials;
		newMaterials.reserve(m_materials.size());

		for (auto &it : m_meshInstances)
		{
			auto materialSid = it.getMaterialSid();

			auto materialIt = m_materials.find(materialSid);

			newMaterials.insert(materialSid, *materialIt);
		}

		newMaterials.swap(m_materials);
	}

	void Scene::removeUnusedMeshes()
	{
		vx::sorted_vector<vx::StringID, const vx::MeshFile*> newMeshes;
		newMeshes.reserve(m_meshes.size());

		for (auto &it : m_meshInstances)
		{
			auto meshSid = it.getMeshSid();

			auto meshIt = m_meshes.find(meshSid);

			newMeshes.insert(meshSid, *meshIt);
		}

		newMeshes.swap(m_meshes);
	}

	Light* Scene::addLight(const Light &light)
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

	u8 Scene::addMesh(vx::StringID sid, const char* name, const vx::MeshFile* pMesh)
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

	u8 Scene::addMaterial(vx::StringID sid, const char* name, Material* pMaterial)
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

	void Scene::addWaypoint(const vx::float3 &position)
	{
#if _VX_EDITOR
		Waypoint w;
		w.position = position;
		w.value = 0.0f;

		m_waypoints.push_back(w);
		++m_waypointCount;

		buildSelectableWaypoints();
#endif
	}

	void Scene::removeWaypoint(const vx::float3 &position)
	{
#if _VX_EDITOR
		bool found = false;
		u32 index = 0;
		for (auto &it : m_selectableWaypoints)
		{
			if (it.m_bounds.contains(position))
			{
				found = true;
				break;
			}

			++index;
		}

		if (found)
		{
			m_waypoints.erase(m_waypoints.begin() + index);
			m_selectableWaypoints.erase(m_selectableWaypoints.begin() + index);
			--m_waypointCount;

			buildSelectableWaypoints();
		}
#endif
	}

	const char* Scene::getMeshInstanceName(const vx::StringID &sid) const
	{
		auto it = m_meshInstances.find(sid);
		if (it == m_meshInstances.end())
			return nullptr;

		return it->getName().c_str();
	}

	const char* Scene::getMaterialName(const vx::StringID &sid) const
	{
		auto it = m_materialNames.find(sid);
		if (it == m_materialNames.end())
			return nullptr;

		return it->c_str();
	}

	const char* Scene::getMeshName(const vx::StringID &sid) const
	{
		auto it = m_meshNames.find(sid);
		if (it == m_meshNames.end())
			return nullptr;

		return it->c_str();
	}

	const char* Scene::getActorName(const vx::StringID &sid) const
	{
		auto it = m_actorNames.find(sid);
		if (it == m_actorNames.end())
			return nullptr;

		return it->c_str();
	}

	vx::StringID Scene::createMeshInstance()
	{
		std::string instanceName = "instance" + std::to_string(m_meshInstances.size());
		auto nameSid = vx::make_sid(instanceName.c_str());

		auto meshSid = *m_meshes.keys();
		auto materialSid = *m_materials.keys();

		vx::Transform transform;
		::MeshInstance instance(nameSid, meshSid, materialSid, transform);
		Editor::MeshInstance editorInstance(instance, std::move(instanceName));

		m_meshInstances.insert(nameSid, editorInstance);

		return nameSid;
	}

	void Scene::removeMeshInstance(const vx::StringID &sid)
	{
		auto it = m_meshInstances.find(sid);
		if (it != m_meshInstances.end())
			m_meshInstances.erase(it);
	}

	bool Scene::renameMeshInstance(const vx::StringID &sid, const char* newName)
	{
		bool result = false;
		auto it = m_meshInstances.find(sid);
		if (it != m_meshInstances.end())
		{
			auto newSid = vx::make_sid(newName);
			::MeshInstance newInstance(newSid, it->getMeshSid(), it->getMaterialSid(), it->getTransform());
			Editor::MeshInstance editrInstance(newInstance, std::string(newName));

			m_meshInstances.erase(it);

			m_meshInstances.insert(newSid, std::move(editrInstance));

			result = true;
		}

		return result;
	}

	const MeshInstance* Scene::getMeshInstance(const vx::StringID &sid) const
	{
		const MeshInstance* p = nullptr;
		auto it = m_meshInstances.find(sid);
		if (it != m_meshInstances.end())
			p = &*it;

		return p;
	}

	MeshInstance* Scene::getMeshInstance(const vx::StringID &sid)
	{
		MeshInstance* p = nullptr;
		auto it = m_meshInstances.find(sid);
		if (it != m_meshInstances.end())
			p = &*it;

		return p;
	}

	const Editor::MeshInstance* Scene::getMeshInstancesEditor() const
	{
		return m_meshInstances.data();
	}

	u32 Scene::getMeshInstanceCount() const
	{
		return m_meshInstances.size();
	}

	Spawn* Scene::getSpawn(const Ray &ray)
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

	Light* Scene::getLight(const Ray &ray)
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

	void Scene::updateLightPositions()
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
}