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
#include <vxLib/File/File.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/Material.h>
#include <fstream>
#include <vxLib/Container/sorted_array.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/Waypoint.h>
#include <vxLib/util/CityHash.h>
#include <vxEngineLib/memcpy.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxLib/File/FileHandle.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/ConverterSceneFileV5.h>

SceneFile::SceneFile(u32 version)
	:Serializable(version),
	m_pMeshInstances(),
	m_pLights(),
	m_pSpawns(),
	m_pActors(),
	m_waypoints(),
	m_joints(),
	m_navMesh(),
	m_meshInstanceCount(0),
	m_lightCount(0),
	m_spawnCount(0),
	m_actorCount(0),
	m_waypointCount(0),
	m_jointCount(0)
{
}

SceneFile::SceneFile(SceneFile &&rhs)
	:Serializable(std::move(rhs)),
	m_pMeshInstances(std::move(rhs.m_pMeshInstances)),
	m_pLights(std::move(rhs.m_pLights)),
	m_pSpawns(std::move(rhs.m_pSpawns)),
	m_pActors(std::move(rhs.m_pActors)),
	m_waypoints(std::move(rhs.m_waypoints)),
	m_joints(std::move(rhs.m_joints)),
	m_navMesh(std::move(rhs.m_navMesh)),
	m_meshInstanceCount(rhs.m_meshInstanceCount),
	m_lightCount(rhs.m_lightCount),
	m_spawnCount(rhs.m_spawnCount),
	m_actorCount(rhs.m_actorCount),
	m_waypointCount(rhs.m_waypointCount),
	m_jointCount(rhs.m_jointCount)
{

}

SceneFile::~SceneFile()
{
}

void SceneFile::swap(SceneFile &other)
{
	if (this != &other)
	{
		std::swap(m_pMeshInstances, other.m_pMeshInstances);
		std::swap(m_pLights, other.m_pLights);
		std::swap(m_pSpawns, other.m_pSpawns);
		std::swap(m_pActors, other.m_pActors);
		std::swap(m_waypoints, other.m_waypoints);
		std::swap(m_joints, other.m_joints);
		m_navMesh.swap(other.m_navMesh);
		std::swap(m_meshInstanceCount, other.m_meshInstanceCount);
		std::swap(m_lightCount, other.m_lightCount);
		std::swap(m_spawnCount, other.m_spawnCount);
		std::swap(m_actorCount, other.m_actorCount);
		std::swap(m_waypointCount, other.m_waypointCount);
		std::swap(m_jointCount, other.m_jointCount);
	}
}

/*const u8* SceneFile::loadVersion3(const u8 *ptr, const u8* last, vx::Allocator* allocator)
{
	ptr = vx::read(m_meshInstanceCount, ptr);
	ptr = vx::read(m_lightCount, ptr);
	ptr = vx::read(m_spawnCount, ptr);
	ptr = vx::read(m_actorCount, ptr);
	ptr = vx::read(m_waypointCount, ptr);

	auto meshInstances = vx::make_unique<MeshInstanceFileOld[]>(m_meshInstanceCount);
	m_pLights = vx::make_unique<Light[]>(m_lightCount);
	m_pSpawns = vx::make_unique<SpawnFile[]>(m_spawnCount);

	ptr = vx::read(meshInstances.get(), ptr, m_meshInstanceCount);
	ptr = vx::read(m_pLights.get(), ptr, m_lightCount);
	ptr = vx::read(m_pSpawns.get(), ptr, m_spawnCount);

	if (m_actorCount != 0)
	{
		m_pActors = vx::make_unique<ActorFile[]>(m_actorCount);

		ptr = vx::read(m_pActors.get(), ptr, m_actorCount);
	}

	if (m_waypointCount != 0)
	{
		m_waypoints = vx::make_unique<Waypoint[]>(m_waypointCount);

		ptr = vx::read(m_waypoints.get(), ptr, m_waypointCount);
	}

	m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(m_meshInstanceCount);
	for (u32 i = 0; i < m_meshInstanceCount; ++i)
	{
		meshInstances[i].convertTo(&m_pMeshInstances[i]);
	}

	VX_ASSERT(ptr < last);

	ptr = m_navMesh.load(ptr);

	VX_ASSERT(ptr <= last);

	return ptr;
}*/

const u8* SceneFile::loadVersion4(const u8 *ptr, const u8* last, vx::Allocator* allocator)
{
	ptr = vx::read(m_meshInstanceCount, ptr);
	ptr = vx::read(m_lightCount, ptr);
	ptr = vx::read(m_spawnCount, ptr);
	ptr = vx::read(m_actorCount, ptr);
	ptr = vx::read(m_waypointCount, ptr);

	m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(m_meshInstanceCount);
	m_pLights = vx::make_unique<Light[]>(m_lightCount);
	m_pSpawns = vx::make_unique<SpawnFile[]>(m_spawnCount);

	auto tmpInstances = vx::make_unique<MeshInstanceFileV4[]>(m_meshInstanceCount);
	ptr = vx::read((u8*)tmpInstances.get(), ptr, sizeof(MeshInstanceFileV4) * m_meshInstanceCount);
	for (u32 i = 0; i < m_meshInstanceCount; ++i)
	{
		m_pMeshInstances[i].convert(tmpInstances[i]);
	}

	ptr = vx::read((u8*)m_pLights.get(), ptr, sizeof(Light) * m_lightCount);
	ptr = vx::read((u8*)m_pSpawns.get(), ptr, sizeof(SpawnFile) *m_spawnCount);

	if (m_actorCount != 0)
	{
		m_pActors = vx::make_unique<ActorFile[]>(m_actorCount);

		ptr = vx::read(m_pActors.get(), ptr, m_actorCount);
	}

	if (m_waypointCount != 0)
	{
		m_waypoints = vx::make_unique<Waypoint[]>(m_waypointCount);

		ptr = vx::read((u8*)m_waypoints.get(), ptr, sizeof(Waypoint) * m_waypointCount);
	}

	VX_ASSERT(ptr < last);

	ptr = m_navMesh.load(ptr);

	VX_ASSERT(ptr <= last);

	return ptr;
}

/*const u8* SceneFile::loadVersion5(const u8 *ptr, const u8* last, vx::Allocator* allocator)
{
	ptr = vx::read(m_meshInstanceCount, ptr);
	ptr = vx::read(m_lightCount, ptr);
	ptr = vx::read(m_spawnCount, ptr);
	ptr = vx::read(m_actorCount, ptr);
	ptr = vx::read(m_waypointCount, ptr);

	m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(m_meshInstanceCount);
	m_pLights = vx::make_unique<Light[]>(m_lightCount);
	m_pSpawns = vx::make_unique<SpawnFile[]>(m_spawnCount);

	ptr = vx::read((u8*)m_pMeshInstances.get(), ptr, sizeof(MeshInstanceFile) * m_meshInstanceCount);
	ptr = vx::read((u8*)m_pLights.get(), ptr, sizeof(Light) * m_lightCount);
	ptr = vx::read((u8*)m_pSpawns.get(), ptr, sizeof(SpawnFile) *m_spawnCount);

	if (m_actorCount != 0)
	{
		m_pActors = vx::make_unique<ActorFile[]>(m_actorCount);

		ptr = vx::read(m_pActors.get(), ptr, m_actorCount);
	}

	if (m_waypointCount != 0)
	{
		m_waypoints = vx::make_unique<Waypoint[]>(m_waypointCount);

		ptr = vx::read((u8*)m_waypoints.get(), ptr, sizeof(Waypoint) * m_waypointCount);
	}

	VX_ASSERT(ptr < last);

	ptr = m_navMesh.load(ptr);

	VX_ASSERT(ptr <= last);

	return ptr;
}*/

const u8* SceneFile::loadVersion6(const u8 *ptr, const u8* last, vx::Allocator* allocator)
{
	ptr = vx::read(m_meshInstanceCount, ptr);
	ptr = vx::read(m_lightCount, ptr);
	ptr = vx::read(m_spawnCount, ptr);
	ptr = vx::read(m_actorCount, ptr);
	ptr = vx::read(m_waypointCount, ptr);
	ptr = vx::read(m_jointCount, ptr);

	m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(m_meshInstanceCount);
	m_pLights = vx::make_unique<Light[]>(m_lightCount);
	m_pSpawns = vx::make_unique<SpawnFile[]>(m_spawnCount);

	ptr = vx::read((u8*)m_pMeshInstances.get(), ptr, sizeof(MeshInstanceFile) * m_meshInstanceCount);
	ptr = vx::read((u8*)m_pLights.get(), ptr, sizeof(Light) * m_lightCount);
	ptr = vx::read((u8*)m_pSpawns.get(), ptr, sizeof(SpawnFile) *m_spawnCount);

	if (m_actorCount != 0)
	{
		m_pActors = vx::make_unique<ActorFile[]>(m_actorCount);

		ptr = vx::read(m_pActors.get(), ptr, m_actorCount);
	}

	if (m_waypointCount != 0)
	{
		m_waypoints = vx::make_unique<Waypoint[]>(m_waypointCount);

		ptr = vx::read((u8*)m_waypoints.get(), ptr, sizeof(Waypoint) * m_waypointCount);
	}

	if (m_jointCount != 0)
	{
		m_joints = vx::make_unique<Joint[]>(m_jointCount);
		ptr = vx::read(m_joints.get(), ptr, m_jointCount);
	}

	VX_ASSERT(ptr < last);

	ptr = m_navMesh.load(ptr);

	VX_ASSERT(ptr <= last);

	return ptr;
}

const u8* SceneFile::loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator)
{
	auto version = getVersion();
	auto last = ptr + size;

	const u8* result = nullptr;
	/*if (version == 3)
	{
		result = loadVersion3(ptr, last, allocator);
	}
	else*/ 
	if (version == 4)
	{
		result = loadVersion4(ptr, last, allocator);
	}
	else if (version == 5)
	{
		result = Converter::SceneFileV5::loadFromMemory(ptr, last, allocator, this);
	}
	else if (version == 6)
	{
		result = loadVersion6(ptr, last, allocator);
	}
	else
	{
		VX_ASSERT(false);
	}

	return result;
}

void SceneFile::saveToFile(vx::File *file) const
{
	file->write(m_meshInstanceCount);
	file->write(m_lightCount);
	file->write(m_spawnCount);
	file->write(m_actorCount);
	file->write(m_waypointCount);
	file->write(m_jointCount);

	file->write(m_pMeshInstances.get(), m_meshInstanceCount);
	file->write(m_pLights.get(), m_lightCount);
	file->write(m_pSpawns.get(), m_spawnCount);
	file->write(m_pActors.get(), m_actorCount);
	file->write(m_waypoints.get(), m_waypointCount);
	file->write(m_joints.get(), m_jointCount);

	m_navMesh.saveToFile(file);
}

u32 SceneFile::getNumMeshInstances() const
{
	return m_meshInstanceCount;
}

/*u64 SceneFile::getCrcVersion3() const
{
	auto navMeshVertexSize = sizeof(vx::float3) * m_navMesh.getVertexCount();
	auto navMeshTriangleSize = sizeof(u16) * m_navMesh.getTriangleCount() * 3;
	auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

	u32 meshInstanceSize = sizeof(MeshInstanceFileOld) * m_meshInstanceCount;
	auto meshInstancesOld = std::make_unique<MeshInstanceFileOld[]>(m_meshInstanceCount);
	for (u32 i = 0; i < m_meshInstanceCount; ++i)
	{
		auto &it = m_pMeshInstances[i];

		meshInstancesOld[i] = MeshInstanceFileOld(it);
	}

	auto lightSize = sizeof(Light) * m_lightCount;
	auto spawnSize = sizeof(SpawnFile) * m_spawnCount;
	auto actorSize = sizeof(ActorFile) * m_actorCount;

	auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize;
	auto ptr = vx::make_unique<u8[]>(totalSize);

	auto offset = 0;
	::memcpy(ptr.get() + offset, meshInstancesOld.get(), meshInstanceSize);
	offset += meshInstanceSize;

	::memcpy(ptr.get() + offset, m_pLights.get(), lightSize);
	offset += lightSize;

	::memcpy(ptr.get() + offset, m_pSpawns.get(), spawnSize);
	offset += spawnSize;

	::memcpy(ptr.get() + offset, m_pActors.get(), actorSize);
	offset += actorSize;

	::memcpy(ptr.get() + offset, m_navMesh.getVertices(), navMeshVertexSize);
	offset += navMeshVertexSize;

	::memcpy(ptr.get() + offset, m_navMesh.getTriangleIndices(), navMeshTriangleSize);
	offset += navMeshTriangleSize;

	return CityHash64((char*)ptr.get(), totalSize);
}*/

u64 SceneFile::getCrcVersion4() const
{
	auto navMeshVertexSize = sizeof(vx::float3) * m_navMesh.getVertexCount();
	auto navMeshTriangleSize = sizeof(u16) * m_navMesh.getTriangleCount() * 3;
	auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

	u32 meshInstanceSize = sizeof(MeshInstanceFileV4) * m_meshInstanceCount;

	auto lightSize = sizeof(Light) * m_lightCount;
	auto spawnSize = sizeof(SpawnFile) * m_spawnCount;
	auto actorSize = sizeof(ActorFile) * m_actorCount;

	auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize;
	auto ptr = vx::make_unique<u8[]>(totalSize);

	auto offset = 0;
	if (meshInstanceSize != 0)
	{
		auto tmpInstances = vx::make_unique<MeshInstanceFileV4[]>(m_meshInstanceCount);
		for (u32 i = 0; i < m_meshInstanceCount; ++i)
		{
			//tmpInstances[i].convert(m_pMeshInstances[i]);
			auto &instance = m_pMeshInstances[i];

			char instanceName[32];
			strncpy(instanceName, instance.getName(), 32);

			char meshName[32];
			strncpy(meshName, instance.getMeshFile(), 32);

			char materialName[32];
			strncpy(materialName, instance.getMaterialFile(), 32);

			char animationName[32];
			strncpy(animationName, instance.getAnimation(), 32);

			auto &transform = instance.getTransform();

			tmpInstances[i] = MeshInstanceFileV4(instanceName, meshName, materialName, animationName, transform);
		}

		::memcpy(ptr.get() + offset, tmpInstances.get(), meshInstanceSize);
		offset += meshInstanceSize;
	}

	if (lightSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pLights.get(), lightSize);
		offset += lightSize;
	}

	if (spawnSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pSpawns.get(), spawnSize);
		offset += spawnSize;
	}

	if (actorSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pActors.get(), actorSize);
		offset += actorSize;
	}

	if (navMeshVertexSize != 0)
	{
		::memcpy(ptr.get() + offset, m_navMesh.getVertices(), navMeshVertexSize);
		offset += navMeshVertexSize;
	}

	if (navMeshTriangleSize != 0)
	{
		::memcpy(ptr.get() + offset, m_navMesh.getTriangleIndices(), navMeshTriangleSize);
		offset += navMeshTriangleSize;
	}

	return CityHash64((char*)ptr.get(), totalSize);
}

/*u64 SceneFile::getCrcVersion5() const
{
	auto navMeshVertexSize = sizeof(vx::float3) * m_navMesh.getVertexCount();
	auto navMeshTriangleSize = sizeof(u16) * m_navMesh.getTriangleCount() * 3;
	auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

	u32 meshInstanceSize = sizeof(MeshInstanceFile) * m_meshInstanceCount;

	auto lightSize = sizeof(Light) * m_lightCount;
	auto spawnSize = sizeof(SpawnFile) * m_spawnCount;
	auto actorSize = sizeof(ActorFile) * m_actorCount;

	auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize;
	auto ptr = vx::make_unique<u8[]>(totalSize);

	auto offset = 0;
	if (meshInstanceSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pMeshInstances.get(), meshInstanceSize);
		offset += meshInstanceSize;
	}

	if (lightSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pLights.get(), lightSize);
		offset += lightSize;
	}

	if (spawnSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pSpawns.get(), spawnSize);
		offset += spawnSize;
	}

	if (actorSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pActors.get(), actorSize);
		offset += actorSize;
	}

	if (navMeshVertexSize != 0)
	{
		::memcpy(ptr.get() + offset, m_navMesh.getVertices(), navMeshVertexSize);
		offset += navMeshVertexSize;
	}

	if (navMeshTriangleSize != 0)
	{
		::memcpy(ptr.get() + offset, m_navMesh.getTriangleIndices(), navMeshTriangleSize);
		offset += navMeshTriangleSize;
	}

	return CityHash64((char*)ptr.get(), totalSize);
}*/

u64 SceneFile::getCrcVersion6() const
{
	auto navMeshVertexSize = sizeof(vx::float3) * m_navMesh.getVertexCount();
	auto navMeshTriangleSize = sizeof(u16) * m_navMesh.getTriangleCount() * 3;
	auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

	u32 meshInstanceSize = sizeof(MeshInstanceFile) * m_meshInstanceCount;

	auto lightSize = sizeof(Light) * m_lightCount;
	auto spawnSize = sizeof(SpawnFile) * m_spawnCount;
	auto actorSize = sizeof(ActorFile) * m_actorCount;
	auto jointSize = sizeof(Joint) * m_jointCount;

	auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize + jointSize;
	auto ptr = vx::make_unique<u8[]>(totalSize);

	auto offset = 0;
	if (meshInstanceSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pMeshInstances.get(), meshInstanceSize);
		offset += meshInstanceSize;
	}

	if (lightSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pLights.get(), lightSize);
		offset += lightSize;
	}

	if (spawnSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pSpawns.get(), spawnSize);
		offset += spawnSize;
	}

	if (actorSize != 0)
	{
		::memcpy(ptr.get() + offset, m_pActors.get(), actorSize);
		offset += actorSize;
	}

	if (navMeshVertexSize != 0)
	{
		::memcpy(ptr.get() + offset, m_navMesh.getVertices(), navMeshVertexSize);
		offset += navMeshVertexSize;
	}

	if (navMeshTriangleSize != 0)
	{
		::memcpy(ptr.get() + offset, m_navMesh.getTriangleIndices(), navMeshTriangleSize);
		offset += navMeshTriangleSize;
	}

	if (jointSize != 0)
	{
		::memcpy(ptr.get() + offset, m_joints.get(), jointSize);
		offset += jointSize;
	}

	return CityHash64((char*)ptr.get(), totalSize);
}

u32 SceneFile::getActorCount() const 
{
	return m_actorCount; 
}

const ActorFile* SceneFile::getActors() const
{
	return m_pActors.get(); 
}

u64 SceneFile::getCrc() const
{
	auto currentVersion = getVersion();

	u64 crc = 0;
	if (currentVersion == 4)
	{
		crc = getCrcVersion4();
	}
	else if (currentVersion == 5)
	{
		crc = Converter::SceneFileV5::getCrc(*this);
	//	crc = getCrcVersion5();
	}
	else if (currentVersion == 6)
	{
		crc = getCrcVersion6();
	}
	else
	{
		VX_ASSERT(false);
	}
	
	return crc;
}

u32 SceneFile::getGlobalVersion()
{
	return 6;
}