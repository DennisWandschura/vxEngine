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

#include <vxEngineLib/ConverterSceneFileV6.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/memcpy.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Joint.h>

namespace Converter
{
	const u8* SceneFileV6::loadFromMemory(const u8 *ptr, const u8* last, vx::Allocator* allocator, SceneFile* sceneFile)
	{
		ptr = vx::read(sceneFile->m_meshInstanceCount, ptr);
		ptr = vx::read(sceneFile->m_lightCount, ptr);
		ptr = vx::read(sceneFile->m_spawnCount, ptr);
		ptr = vx::read(sceneFile->m_actorCount, ptr);
		ptr = vx::read(sceneFile->m_waypointCount, ptr);
		ptr = vx::read(sceneFile->m_jointCount, ptr);

		sceneFile->m_pLights = vx::make_unique<Light[]>(sceneFile->m_lightCount);
		sceneFile->m_pSpawns = vx::make_unique<SpawnFile[]>(sceneFile->m_spawnCount);

		if (sceneFile->m_meshInstanceCount != 0)
		{
			auto tmpInstances = vx::make_unique<MeshInstanceFileV5[]>(sceneFile->m_meshInstanceCount);
			ptr = vx::read(tmpInstances.get(), ptr, sceneFile->m_meshInstanceCount);

			sceneFile->m_pMeshInstances = vx::make_unique<MeshInstanceFileV8[]>(sceneFile->m_meshInstanceCount);
			for (u32 i = 0; i < sceneFile->m_meshInstanceCount; ++i)
			{
				sceneFile->m_pMeshInstances[i].convert(tmpInstances[i]);
			}
		}

		ptr = vx::read(sceneFile->m_pLights.get(), ptr, sceneFile->m_lightCount);
		ptr = vx::read(sceneFile->m_pSpawns.get(), ptr, sceneFile->m_spawnCount);

		if (sceneFile->m_actorCount != 0)
		{
			sceneFile->m_pActors = vx::make_unique<ActorFile[]>(sceneFile->m_actorCount);

			ptr = vx::read(sceneFile->m_pActors.get(), ptr, sceneFile->m_actorCount);
		}

		if (sceneFile->m_waypointCount != 0)
		{
			sceneFile->m_waypoints = vx::make_unique<Waypoint[]>(sceneFile->m_waypointCount);
			ptr = vx::read(sceneFile->m_waypoints.get(), ptr, sceneFile->m_waypointCount);
		}

		auto jointCount = sceneFile->m_jointCount;
		if (jointCount != 0)
		{
			auto tmp = vx::make_unique<JointV6[]>(jointCount);
			ptr = vx::read(tmp.get(), ptr, jointCount);
			sceneFile->m_joints = vx::make_unique<Joint[]>(jointCount);

			for (u32 i = 0; i < jointCount; ++i)
			{
				sceneFile->m_joints[i].p0 = tmp[i].p0;
				sceneFile->m_joints[i].p1 = tmp[i].p1;
				sceneFile->m_joints[i].q0 = tmp[i].q0;
				sceneFile->m_joints[i].q1 = tmp[i].q1;
				sceneFile->m_joints[i].sid0 = tmp[i].sid0;
				sceneFile->m_joints[i].sid1 = tmp[i].sid1;
				sceneFile->m_joints[i].limitEnabled = 0;
			}
		}

		VX_ASSERT(ptr < last);

		ptr = sceneFile->m_navMesh.load(ptr);

		VX_ASSERT(ptr <= last);

		return ptr;
	}

	u64 SceneFileV6::getCrc(const SceneFile &sceneFile)
	{
		auto navVertexCount = sceneFile.m_navMesh.getVertexCount();
		auto navIndexCount = sceneFile.m_navMesh.getTriangleCount() * 3;
		auto navMeshVertexSize = sizeof(vx::float3) * navVertexCount;
		auto navMeshTriangleSize = sizeof(u16) * navIndexCount;
		auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

		u32 meshInstanceSize = sizeof(MeshInstanceFileV5) * sceneFile.m_meshInstanceCount;

		auto lightSize = sizeof(Light) * sceneFile.m_lightCount;
		auto spawnSize = sizeof(SpawnFile) * sceneFile.m_spawnCount;
		auto actorSize = sizeof(ActorFile) * sceneFile.m_actorCount;
		auto jointSize = sizeof(JointV6) * sceneFile.m_jointCount;

		auto sceneJoints = sceneFile.m_joints.get();
		auto jointCount = sceneFile.m_jointCount;
		auto tmp = vx::make_unique<JointV6[]>(jointCount);
		for (u32 i = 0; i < jointCount; ++i)
		{
			tmp[i].p0 = sceneJoints[i].p0;
			tmp[i].p1 = sceneJoints[i].p1;
			tmp[i].q0 = sceneJoints[i].q0;
			tmp[i].q1 = sceneJoints[i].q1;
			tmp[i].sid0 = sceneJoints[i].sid0;
			tmp[i].sid1 = sceneJoints[i].sid1;
		}

		auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize + jointSize;
		auto ptr = vx::make_unique<u8[]>(totalSize);
		auto current = ptr.get();

		if (meshInstanceSize != 0)
		{
			auto tmpMeshInstances = vx::make_unique<MeshInstanceFileV5[]>(sceneFile.m_meshInstanceCount);
			for (u32 i = 0; i < sceneFile.m_meshInstanceCount; ++i)
			{
				tmpMeshInstances[i].convert(sceneFile.m_pMeshInstances[i]);
			}
			current = vx::write(current, tmpMeshInstances.get(), sceneFile.m_meshInstanceCount);
		}
		
		current = vx::write(current, sceneFile.m_pLights.get(), sceneFile.m_lightCount);
		current = vx::write(current, sceneFile.m_pSpawns.get(), sceneFile.m_spawnCount);
		current = vx::write(current, sceneFile.m_pActors.get(), sceneFile.m_actorCount);
		current = vx::write(current, sceneFile.m_navMesh.getVertices(), navVertexCount);
		current = vx::write(current, sceneFile.m_navMesh.getTriangleIndices(), navIndexCount);
		current = vx::write(current, tmp.get(), jointCount);

		auto last = ptr.get() + totalSize;
		VX_ASSERT(current == last);

		return CityHash64((char*)ptr.get(), totalSize);
	}
}