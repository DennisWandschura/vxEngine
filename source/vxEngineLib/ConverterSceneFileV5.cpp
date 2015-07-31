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

#include <vxEngineLib/ConverterSceneFileV5.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/memcpy.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Actor.h>

namespace Converter
{
	const u8* SceneFileV5::loadFromMemory(const u8 *ptr, const u8* last, vx::Allocator* allocator, SceneFile* sceneFile)
	{
		ptr = vx::read(sceneFile->m_meshInstanceCount, ptr);
		ptr = vx::read(sceneFile->m_lightCount, ptr);
		ptr = vx::read(sceneFile->m_spawnCount, ptr);
		ptr = vx::read(sceneFile->m_actorCount, ptr);
		ptr = vx::read(sceneFile->m_waypointCount, ptr);

		sceneFile->m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(sceneFile->m_meshInstanceCount);
		sceneFile->m_pLights = vx::make_unique<Light[]>(sceneFile->m_lightCount);
		sceneFile->m_pSpawns = vx::make_unique<SpawnFile[]>(sceneFile->m_spawnCount);

		ptr = vx::read(sceneFile->m_pMeshInstances.get(), ptr, sceneFile->m_meshInstanceCount);
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

		VX_ASSERT(ptr < last);

		ptr = sceneFile->m_navMesh.load(ptr);

		VX_ASSERT(ptr <= last);

		return ptr;
	}

	u64 SceneFileV5::getCrc(const SceneFile &sceneFile)
	{
		auto navMeshVertexSize = sizeof(vx::float3) * sceneFile.m_navMesh.getVertexCount();
		auto navMeshTriangleSize = sizeof(u16) * sceneFile.m_navMesh.getTriangleCount() * 3;
		auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

		u32 meshInstanceSize = sizeof(MeshInstanceFile) * sceneFile.m_meshInstanceCount;

		auto lightSize = sizeof(Light) * sceneFile.m_lightCount;
		auto spawnSize = sizeof(SpawnFile) * sceneFile.m_spawnCount;
		auto actorSize = sizeof(ActorFile) * sceneFile.m_actorCount;

		auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize;
		auto ptr = vx::make_unique<u8[]>(totalSize);

		auto current = ptr.get();
		if (meshInstanceSize != 0)
		{
			::memcpy(current, sceneFile.m_pMeshInstances.get(), meshInstanceSize);
			current += meshInstanceSize;
		}

		if (lightSize != 0)
		{
			::memcpy(current, sceneFile.m_pLights.get(), lightSize);
			current += lightSize;
		}

		if (spawnSize != 0)
		{
			::memcpy(current, sceneFile.m_pSpawns.get(), spawnSize);
			current += spawnSize;
		}

		if (actorSize != 0)
		{
			::memcpy(current, sceneFile.m_pActors.get(), actorSize);
			current += actorSize;
		}

		if (navMeshVertexSize != 0)
		{
			::memcpy(current, sceneFile.m_navMesh.getVertices(), navMeshVertexSize);
			current += navMeshVertexSize;
		}

		if (navMeshTriangleSize != 0)
		{
			::memcpy(current, sceneFile.m_navMesh.getTriangleIndices(), navMeshTriangleSize);
			current += navMeshTriangleSize;
		}


		auto last = ptr.get() + totalSize;
		VX_ASSERT(current == last);

		return CityHash64((char*)ptr.get(), totalSize);
	}
}