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
#include <vxEngineLib/Reference.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Joint.h>

namespace EditorSceneCpp
{
	const u32 g_invalidId = 0xffffffff;
}

namespace Editor
{
	SceneParams::~SceneParams()
	{

	}

	Scene::Scene()
		:m_meshInstances(),
		m_selectableLights(),
		m_selectableSpawns(),
		m_selectableWaypoints(),
		m_selectableJoints(),
		m_materialNames(),
		m_meshNames(),
		m_actorNames(),
		m_animationNames(),
		m_spawnHumanId(EditorSceneCpp::g_invalidId)
	{

	}

	Scene::Scene(SceneParams &&params)
		: SceneBase(params.m_baseParams),
		m_meshInstances(std::move(params.m_meshInstances)),
		m_selectableLights(),
		m_selectableWaypoints(),
		m_selectableJoints(),
		m_materialNames(std::move(params.m_materialNames)),
		m_meshNames(std::move(params.m_meshNames)),
		m_actorNames(std::move(params.m_actorNames))
	{
		buildSelectableLights();

		buildSelectableSpawns();

		buildSelectableJoints();
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
			m_selectableJoints.swap(rhs.m_selectableJoints);
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
			auto &light = m_lights[i];

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
			bounds.max = waypoint.position + vx::float3(0.05f);
			bounds.min = waypoint.position - vx::float3(0.05f);

			SelectableWrapper<Waypoint> selectLight;
			selectLight.m_bounds = bounds;
			selectLight.m_ptr = &waypoint;

			m_selectableWaypoints.push_back(selectLight);
		}
	}

	void Scene::buildSelectableSpawns()
	{
		m_selectableSpawns.clear();
		m_selectableSpawns.reserve(m_spawnCount);

		for (u32 i = 0; i < m_spawnCount; ++i)
		{
			auto &spawn = m_pSpawns[i];

			AABB bounds;
			bounds.max = spawn.position + vx::float3(0.1f);
			bounds.min = spawn.position - vx::float3(0.1f);

			if (spawn.type == PlayerType::Human)
				m_spawnHumanId = spawn.id;

			std::pair<AABB, u32> selected;
			selected.first = bounds;
			selected.second = spawn.id;

			m_selectableSpawns.push_back(selected);
		}
	}

	void Scene::buildSelectableJoints()
	{
		m_selectableJoints.clear();

		auto jointCount = m_joints.size();
		m_selectableJoints.reserve(jointCount);

		for (u32 i = 0; i < jointCount; ++i)
		{
			auto &joint = m_joints[i];

			auto p0 = joint.p0;
			auto p1 = joint.p1;
			if (joint.sid0.value != 0)
			{
				auto it = m_meshInstances.find(joint.sid0);
				p0 = it->getTransform().m_translation;
			}

			if (joint.sid1.value != 0)
			{
				auto it = m_meshInstances.find(joint.sid1);
				p1 = it->getTransform().m_translation;
			}

			auto vmin = vx::min(p0, p1);
			auto vmax = vx::max(p0, p1);

			SelectableWrapperIndex selectable;
			selectable.m_bounds.max = vmax + vx::float3(0.1f);
			selectable.m_bounds.min = vmin - vx::float3(0.1f);
			selectable.m_index = i;

			m_selectableJoints.push_back(selectable);
		}
	}

	void Scene::reset()
	{
		SceneBase::reset();
		m_meshInstances.clear();
		m_selectableLights.clear();
		m_selectableSpawns.clear();
		m_selectableWaypoints.clear();
		m_selectableJoints.clear();
		m_materialNames.clear();
		m_meshNames.clear();
		m_actorNames.clear();
		m_animationNames.clear();
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
		copySortedVector(&dst->m_animationNames, m_animationNames);
	}

	void Scene::sortMeshInstances()
	{
	}

	void Scene::removeUnusedMaterials()
	{
	/*	vx::sorted_vector<vx::StringID, Reference<Material>> newMaterials;
		newMaterials.reserve(m_materials.size());

		for (auto &it : m_meshInstances)
		{
			auto materialSid = it.getMaterialSid();

			auto materialIt = m_materials.find(materialSid);

			newMaterials.insert(materialSid, *materialIt);
		}

		newMaterials.swap(m_materials);*/
	}

	void Scene::removeUnusedMeshes()
	{
		vx::sorted_vector<vx::StringID, Reference<vx::MeshFile>> newMeshes;
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
		m_lights.push_back(light);
		++m_lightCount;

		buildSelectableLights();

		return &m_lights.back();
	}

	u8 Scene::addMesh(vx::StringID sid, const char* name, const Reference<vx::MeshFile> &mesh)
	{
		u8 result = 0;
		auto it = m_meshes.find(sid);
		if (it == m_meshes.end())
		{
			char buffer[32];
			strcpy_s(buffer, name);

			m_meshNames.insert(sid, buffer);

			m_meshes.insert(sid, mesh);
			result = 1;
		}

		return result;
	}

	u8 Scene::addMaterial(vx::StringID sid, const char* name, const Reference<Material> &material)
	{
		u8 result = 0;
		auto it = m_materials.find(sid);
		if (it == m_materials.end())
		{
			char buffer[32];
			strcpy_s(buffer, name);
			m_materialNames.insert(sid, buffer);
			m_materials.insert(sid, material);
			result = 1;
		}

		return result;
	}

	void Scene::addWaypoint(const vx::float3 &position)
	{
		Waypoint w;
		w.position = position;
		w.value = 0.0f;

		m_waypoints.push_back(w);
		++m_waypointCount;

		buildSelectableWaypoints();
	}

	void Scene::removeWaypoint(const vx::float3 &position)
	{
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
	}

	void Scene::addAnimation(const vx::StringID &sid, std::string &&name)
	{
		printf("Scene::addAnimation: %llu\n", sid.value);
		auto it = m_animationNames.find(sid);
		if (it == m_animationNames.end())
		{
			auto s = sid;
			m_animationNames.insert(std::move(s), std::move(name));
		}
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

	const char* Scene::getAnimationName(const vx::StringID &sid) const
	{
		auto it = m_animationNames.find(sid);
		if (it == m_animationNames.end())
			return nullptr;

		return it->c_str();
	}

	u32 Scene::getAnimationCount() const
	{
		return m_animationNames.size();
	}

	const char* Scene::getAnimationNameIndex(u32 i) const
	{
		return m_animationNames[i].c_str();
	}

	u64 Scene::getAnimationSidIndex(u32 i) const
	{
		return m_animationNames.keys()[i].value;
	}

	vx::StringID Scene::createMeshInstance()
	{
		std::string instanceName = "instance" + std::to_string(m_meshInstances.size());
		auto nameSid = vx::make_sid(instanceName.c_str());

		auto meshSid = *m_meshes.keys();
		auto material = m_materials[0];

		MeshInstanceDesc desc;
		desc.nameSid = nameSid;
		desc.meshSid = meshSid;
		desc.material = material;
		desc.rigidBodyType = PhysxRigidBodyType::Static;

		::MeshInstance instance(desc);
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

			MeshInstanceDesc desc;
			desc.nameSid = newSid;
			desc.meshSid = it->getMeshSid();
			desc.material = it->getMaterial();
			desc.animationSid = it->getAnimationSid();
			desc.transform = it->getTransform();
			desc.rigidBodyType = it->getRigidBodyType();

			::MeshInstance newInstance(desc);
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

	const vx::sorted_vector<vx::StringID, MeshInstance>& Scene::getSortedMeshInstances() const
	{
		return m_meshInstances;
	}

	void Scene::addSpawn(Spawn &&newSpawn)
	{
		auto it = m_pSpawns.insert(newSpawn.id, std::move(newSpawn));
		++m_spawnCount;

		AABB bounds;
		bounds.max = it->position + vx::float3(0.1f);
		bounds.min = it->position - vx::float3(0.1f);

		std::pair<AABB, u32> selected;
		selected.first = bounds;
		selected.second = it->id;

		m_selectableSpawns.push_back(selected);
	}

	u32 Scene::getSpawnId(const Ray &ray) const
	{
		u32 result = EditorSceneCpp::g_invalidId;

		f32 a, b;
		for (auto &it : m_selectableSpawns)
		{
			if (it.first.intersects(ray, &a, &b))
			{
				result = it.second;
				break;
			}
		}

		return result;
	}

	const Spawn* Scene::getSpawn(u32 id) const
	{
		const Spawn* result = nullptr;

		auto it = m_pSpawns.find(id);
		if (it != m_pSpawns.end())
			result = &*it;
		return result;
	}

	void Scene::setSpawnPosition(u32 id, const vx::float3 &position)
	{
		auto it = m_pSpawns.find(id);
		if (it != m_pSpawns.end())
		{
			it->position = position;

			buildSelectableSpawns();
		}
	}

	void Scene::setSpawnType(u32 id, u32 type)
	{
		auto it = m_pSpawns.find(id);
		if (it != m_pSpawns.end())
		{
			auto playerType = (PlayerType)type;
			it->type = playerType;

			auto otherId = it->id;

			if (playerType == PlayerType::Human && m_spawnHumanId != EditorSceneCpp::g_invalidId && m_spawnHumanId != otherId)
			{
				it = m_pSpawns.find(m_spawnHumanId);
				it->type = PlayerType::AI;

				m_spawnHumanId = otherId;
			}
		}
	}

	u32 Scene::getSpawnHumanId() const
	{
		return m_spawnHumanId;
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
			auto &light = m_lights[i];

			AABB bounds;
			bounds.max = light.m_position + vx::float3(0.1f);
			bounds.min = light.m_position - vx::float3(0.1f);

			m_selectableLights[i].m_bounds = bounds;
		}
	}

	void Scene::addJoint(const Joint &joint)
	{
		m_joints.push_back(joint);

		buildSelectableJoints();
	}

	void Scene::eraseJoint(u32 i)
	{
		auto size = m_joints.size();
		if (i < size)
		{
			auto it = m_joints.begin() + i;
			m_joints.erase(it);

			buildSelectableJoints();
		}
	}

	Joint* Scene::getJoint(const Ray &ray, u32* index)
	{
		Joint* result = nullptr;

		f32 a, b;
		for (auto &it : m_selectableJoints)
		{
			if (it.m_bounds.intersects(ray, &a, &b))
			{
				result = &m_joints[it.m_index];
				*index = it.m_index;
				break;
			}
		}

		return result;
	}

	void Scene::setJointPosition0(u32 index, const vx::float3 &p)
	{
		auto &joint = m_joints[index];
		joint.p0 = p;

		buildSelectableJoints();
	}

	void Scene::setJointPosition1(u32 index, const vx::float3 &p)
	{
		auto &joint = m_joints[index];
		joint.p1 = p;

		buildSelectableJoints();
	}

	void Scene::setJointBody0(u32 index, u64 sid)
	{
		auto &joint = m_joints[index];
		joint.sid0.value = sid;
	}

	void Scene::setJointBody1(u32 index, u64 sid)
	{
		auto &joint = m_joints[index];
		joint.sid1.value = sid;
	}
}