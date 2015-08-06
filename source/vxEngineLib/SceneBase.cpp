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
#include <vxEngineLib/SceneBase.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/copy.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Reference.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Joint.h>

SceneBaseParams::SceneBaseParams()
{

}

SceneBaseParams::~SceneBaseParams()
{

}

SceneBase::SceneBase()
	:m_lights(),
	m_materials(),
	m_meshes(),
	m_pSpawns(),
	m_actors(),
	m_waypoints(),
	m_animations(),
	m_joints(),
	m_navMesh(),
	m_indexCount(0),
	m_spawnCount(0),
	m_waypointCount(0)
{
}

SceneBase::SceneBase(SceneBase &&rhs)
	: m_lights(std::move(rhs.m_lights)),
	m_materials(std::move(rhs.m_materials)),
	m_meshes(std::move(rhs.m_meshes)),
	m_pSpawns(std::move(rhs.m_pSpawns)),
	m_actors(std::move(rhs.m_actors)),
	m_waypoints(std::move(rhs.m_waypoints)),
	m_animations(std::move(rhs.m_animations)),
	m_joints(std::move(rhs.m_joints)),
	m_navMesh(std::move(rhs.m_navMesh)),
	m_lightCount(rhs.m_lightCount),
	m_vertexCount(rhs.m_vertexCount),
	m_indexCount(rhs.m_indexCount),
	m_spawnCount(rhs.m_spawnCount),
	m_waypointCount(rhs.m_waypointCount)
{
}

SceneBase::SceneBase(SceneBaseParams &params)
	: m_lights(std::move(params.m_lights)),
	m_materials(std::move(params.m_materials)),
	m_meshes(std::move(params.m_meshes)),
	m_pSpawns(std::move(params.m_pSpawns)),
	m_actors(std::move(params.m_actors)),
	m_waypoints(std::move(params.m_waypoints)),
	m_animations(std::move(params.m_animations)),
	m_joints(std::move(params.m_joints)),
	m_navMesh(std::move(params.m_navMesh)),
	m_lightCount(params.m_lightCount),
	m_vertexCount(params.m_vertexCount),
	m_indexCount(params.m_indexCount),
	m_spawnCount(params.m_spawnCount),
	m_waypointCount(params.m_waypointCount)
{
}

SceneBase& SceneBase::operator = (SceneBase &&rhs)
{
	if (this != &rhs)
	{
		m_lights = std::move(rhs.m_lights);
		m_materials = std::move(rhs.m_materials);
		m_meshes = std::move(rhs.m_meshes);
		m_pSpawns = std::move(rhs.m_pSpawns);
		m_actors = std::move(rhs.m_actors);
		m_waypoints = std::move(rhs.m_waypoints);
		m_animations = std::move(rhs.m_animations);
		m_joints = std::move(rhs.m_joints);
		std::swap(m_navMesh, rhs.m_navMesh);
		m_lightCount = rhs.m_lightCount;
		m_vertexCount = rhs.m_vertexCount;
		m_indexCount = rhs.m_indexCount;
		m_spawnCount = rhs.m_spawnCount;
		m_waypointCount = rhs.m_waypointCount;
	}

	return *this;
}

SceneBase::~SceneBase()
{
}

void SceneBase::reset()
{
	m_lights.clear();
	m_waypoints.clear();
	m_pSpawns.clear();
	m_joints.clear();

	m_materials.clear();
	m_meshes.clear();
	m_actors.clear();
	m_navMesh.reset();
	m_animations.clear();
	m_lightCount = 0;
	m_vertexCount = 0;
	m_indexCount = 0;
	m_spawnCount = 0;
	m_waypointCount=0;
}

void SceneBase::copy(SceneBase *dst) const
{
	dst->m_lights = m_lights;

	copySortedVector(&dst->m_materials, m_materials);
	copySortedVector(&dst->m_meshes, m_meshes);
	copySortedVector(&dst->m_actors, m_actors);

	dst->m_pSpawns = m_pSpawns;

	dst->m_waypoints = m_waypoints;
	dst->m_animations = m_animations;
	dst->m_joints = m_joints;

	m_navMesh.copy(&dst->m_navMesh);

	dst->m_lightCount = m_lightCount;
	dst->m_vertexCount = m_vertexCount;
	dst->m_indexCount = m_indexCount;
	dst->m_spawnCount = m_spawnCount;
	dst->m_waypointCount = m_waypointCount;
}

const Light* SceneBase::getLights() const
{
	return m_lights.data();
}

u32 SceneBase::getLightCount() const
{
	return m_lightCount;
}

const Reference<Material>* SceneBase::getMaterials() const
{
	return m_materials.data();
}

u32 SceneBase::getMaterialCount() const
{
	return m_materials.size();
}

const Reference<Material>* SceneBase::getMaterial(const vx::StringID &sid) const
{
	const Reference<Material>* result = nullptr;

	auto it = m_materials.find(sid);
	if (it != m_materials.end())
	{
		result = &(*it);
	}

	return result;
}

const vx::sorted_vector<vx::StringID, Reference<vx::MeshFile>>& SceneBase::getMeshes() const
{
	return m_meshes;
}

u32 SceneBase::getVertexCount() const
{
	return m_vertexCount;
}

const Spawn* SceneBase::getSpawns() const
{
	return m_pSpawns.data();
}

u32 SceneBase::getSpawnCount() const
{
	return m_spawnCount;
}

const vx::sorted_vector<vx::StringID, Actor>& SceneBase::getActors() const
{
	return m_actors;
}

const Reference<vx::Animation>* SceneBase::getAnimations() const
{
	return m_animations.data();
}

u32 SceneBase::getAnimationCount() const
{
	return m_animations.size();
}

NavMesh& SceneBase::getNavMesh()
{
	return m_navMesh;
}

const NavMesh& SceneBase::getNavMesh() const
{
	return m_navMesh;
}

const Waypoint* SceneBase::getWaypoints() const
{
	return m_waypoints.data();
}

u32 SceneBase::getWaypointCount() const
{
	return m_waypointCount;
}

const Joint* SceneBase::getJoints() const
{
	return m_joints.data();
}

u32 SceneBase::getJointCount() const
{
	return static_cast<u32>(m_joints.size());
}