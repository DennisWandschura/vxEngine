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
#include <vxLib/Container/sorted_array.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/Waypoint.h>
#include <vxLib/util/CityHash.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxLib/File/FileHandle.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/ConverterSceneFileV5.h>
#include <vxEngineLib/ConverterSceneFileV6.h>
#include "ConverterSceneFileV7.h"
#include "ConverterSceneFileV8.h"

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

const u8* SceneFile::loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator)
{
	auto version = getVersion();
	auto last = ptr + size;

	const u8* result = nullptr;

	if (version == 5)
	{
		result = Converter::SceneFileV5::loadFromMemory(ptr, last, allocator, this);
	}
	else if (version == 6)
	{
		result = Converter::SceneFileV6::loadFromMemory(ptr, last, allocator, this);
	}
	else if (version == 7)
	{
		result = Converter::SceneFileV7::loadFromMemory(ptr, last, allocator, this);
	}
	else if (version == 8)
	{
		result = Converter::SceneFileV8::loadFromMemory(ptr, last, allocator, this);
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

u64 SceneFile::getCrc(u32 version) const
{
	u64 crc = 0;

	switch (version)
	{
	case 5:
		crc = Converter::SceneFileV5::getCrc(*this);
		break;
	case 6:
		crc = Converter::SceneFileV6::getCrc(*this);
		break;
	case 7:
		crc = Converter::SceneFileV7::getCrc(*this);
		break;
	case 8:
		crc = Converter::SceneFileV8::getCrc(*this);
		break;
	default:
		VX_ASSERT(false);
		break;
	}

	return crc;
}

u64 SceneFile::getCrc() const
{
	auto currentVersion = getVersion();

	return getCrc(currentVersion);
}

u32 SceneFile::getGlobalVersion()
{
	return 8;
}