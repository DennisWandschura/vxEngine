#pragma once

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

#include <vxEngineLib/SceneFile.h>

namespace Converter
{
	class SceneFile : public ::SceneFile
	{
		typedef ::SceneFile MyBase;

	public:
		SceneFile(MyBase &&rhs) :MyBase(std::move(rhs)) {}
		~SceneFile() {}

		void swap(MyBase &other)
		{
			MyBase::swap(other);
		}

		void setMeshInstances(std::unique_ptr<MeshInstanceFileV8[]> &&instances, u32 count)
		{
			m_pMeshInstances = std::move(instances);
			m_meshInstanceCount = count;
		}

		void setLights(std::unique_ptr<Light[]> &&lights, u32 count)
		{
			m_pLights = std::move(lights);
			m_lightCount = count;
		}

		void setSpawns(std::unique_ptr<SpawnFile[]> &&spawns, u32 count)
		{
			m_pSpawns = std::move(spawns);
			m_spawnCount = count;
		}

		void setActors(std::unique_ptr<ActorFile[]> &&actors, u32 count)
		{
			m_pActors = std::move(actors);
			m_actorCount = count;
		}

		void setWaypoints(std::unique_ptr<Waypoint[]> &&waypoints, u32 count)
		{
			m_waypoints = std::move(waypoints);
			m_waypointCount = count;
		}

		void setJoints(std::unique_ptr<Joint[]> &&joints, u32 count)
		{
			m_joints = std::move(joints);
			m_jointCount = count;
		}

		void setNavMesh(const NavMesh &navMesh)
		{
			navMesh.copy(&m_navMesh);
		}

		const MeshInstanceFileV8* getMeshInstances() const { return m_pMeshInstances.get(); }
		u32 getMeshInstanceCount() const { return m_meshInstanceCount; }

		u32 getLightCount() const { return m_lightCount; }
		const Light* getLights() const { return m_pLights.get(); }

		u32 getSpawnCount() const { return m_spawnCount; }
		const SpawnFile* getSpawns() const { return m_pSpawns.get(); }

		u32 getActorCount() const { return m_actorCount; }
		const ActorFile* getActors() const { return m_pActors.get(); }

		u32 getWaypointCount() const { return m_waypointCount; }
		const Waypoint* getWaypoints() const { return m_waypoints.get(); }

		u32 getJointCount() const { return m_jointCount; }
		const Joint* getJoints() const { return m_joints.get(); }

		const NavMesh& getNavMesh() const { return m_navMesh; }
		NavMesh& getNavMesh() { return m_navMesh; }
	};
}