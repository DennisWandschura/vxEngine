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
#include "SceneFile.h"
#include "utility.h"
#include <vxLib/File/File.h>
#include "MeshInstance.h"
#include "Material.h"
#include <fstream>
#include <vxLib/Container/sorted_array.h>
#include "Actor.h"
#include "Spawn.h"
#include <vxEngineLib/MeshFile.h>
#include "EditorScene.h"
#include "Light.h"
#include "Scene.h"
#include "Waypoint.h"
#include <vxLib/util/CityHash.h>

#include <string>

struct SceneFile::CreateSceneMeshInstancesDesc
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
	MeshInstance* pMeshInstances;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
	vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	vx::sorted_vector<vx::StringID, std::string>* sceneMeshInstanceNames;
};

struct SceneFile::CreateSceneActorsDesc
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
	vx::sorted_vector<vx::StringID, Actor>* sceneActors;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
	vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
};

struct SceneFile::CreateSceneShared
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
	MeshInstance* pMeshInstances;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
	vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	vx::sorted_vector<vx::StringID, Actor>* sceneActors;
	vx::sorted_vector<vx::StringID, std::string>* sceneMeshInstanceNames;
	Spawn* sceneSpawns;
	u32* vertexCount;
	u32* indexCount;
};

SceneFile::SceneFile()
	:m_meshInstanceCount(0),
	m_lightCount(0),
	m_spawnCount(0),
	m_actorCount(0)
{
}

SceneFile::~SceneFile()
{
}

const u8* SceneFile::loadFromMemory(const u8 *ptr, u32 version, vx::Allocator* allocator)
{
	ptr = vx::read(m_meshInstanceCount, ptr);
	ptr = vx::read(m_lightCount, ptr);
	ptr = vx::read(m_spawnCount, ptr);
	ptr = vx::read(m_actorCount, ptr);

	m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(m_meshInstanceCount);
	m_pLights = vx::make_unique<Light[]>(m_lightCount);
	m_pSpawns = vx::make_unique<SpawnFile[]>(m_spawnCount);

	ptr = vx::read(m_pMeshInstances.get(), ptr, m_meshInstanceCount);
	ptr = vx::read(m_pLights.get(), ptr, m_lightCount);
	ptr = vx::read(m_pSpawns.get(), ptr, m_spawnCount);

	if (m_actorCount != 0)
	{
		m_pActors = vx::make_unique<ActorFile[]>(m_actorCount);

		ptr = vx::read(m_pActors.get(), ptr, m_actorCount);
	}

	return m_navMesh.load(ptr);
}

bool SceneFile::saveToFile(const char *file) const
{
	bool result = false;
	vx::File f;
	if (f.create(file, vx::FileAccess::Write))
	{
		saveToFile(&f);
	}
	else
	{
		printf("SceneFile::saveToFile: Could not create file %s\n", file);
	}
	f.close();

	return result;
}

bool SceneFile::saveToFile(vx::File *file) const
{
	if (!m_navMesh.isValid())
	{
		printf("SceneFile::saveToFile: Nav Mesh not valid !\n");
		return false;
	}

	file->write(m_meshInstanceCount);
	file->write(m_lightCount);
	file->write(m_spawnCount);
	file->write(m_actorCount);

	file->write(m_pMeshInstances.get(), m_meshInstanceCount);
	file->write(m_pLights.get(), m_lightCount);
	file->write(m_pSpawns.get(), m_spawnCount);
	file->write(m_pActors.get(), m_actorCount);

	m_navMesh.saveToFile(file);

	return true;
}

const std::unique_ptr<MeshInstanceFile[]>& SceneFile::getMeshInstances() const noexcept
{
	return m_pMeshInstances;
}

u32 SceneFile::getNumMeshInstances() const noexcept
{
	return m_meshInstanceCount;
}

bool SceneFile::createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc)
{
	for (auto i = 0u; i < m_meshInstanceCount; ++i)
	{
		std::string genName = "instance" + std::to_string(i);

		auto &instance = m_pMeshInstances[i];
		auto name = instance.getName();
		auto meshFile = instance.getMeshFile();
		auto materialFile = instance.getMaterialFile();

		if (name[0] == '\0')
			name = genName.c_str();

		auto sidName = vx::make_sid(name);
		auto sidMesh = vx::make_sid(meshFile);
		auto itMesh = desc.sortedMeshes->find(sidMesh);
		auto sidMaterial = vx::make_sid(materialFile);
		auto itMaterial = desc.sortedMaterials->find(sidMaterial);

		if (itMesh == desc.sortedMeshes->end() || itMaterial == desc.sortedMaterials->end())
		{
			return false;
		}

		desc.sceneMeshes->insert(sidMesh, *itMesh);
		desc.sceneMaterials->insert(sidMaterial, *itMaterial);

		desc.pMeshInstances[i] = MeshInstance(sidName, sidMesh, sidMaterial, instance.getTransform());

		if (desc.sceneMeshInstanceNames)
		{
			desc.sceneMeshInstanceNames->insert(sidName, std::string(name));
		}
	}

	return true;
}

bool SceneFile::createSceneActors(const CreateSceneActorsDesc &desc)
{
	if (m_actorCount != 0)
	{
		//pActors = vx::make_unique<Actor[]>(m_actorCount);
		desc.sceneActors->reserve(m_actorCount);
		for (auto i = 0u; i < m_actorCount; ++i)
		{
			auto &actor = m_pActors[i];

			auto sidMesh = vx::make_sid(actor.m_mesh);
			auto itMesh = desc.sortedMeshes->find(sidMesh);

			auto sidMaterial = vx::make_sid(actor.m_material);
			auto itMaterial = desc.sortedMaterials->find(sidMaterial);

			if (itMesh == desc.sortedMeshes->end() || itMaterial == desc.sortedMaterials->end())
			{
				return false;
			}

			desc.sceneMeshes->insert(sidMesh, *itMesh);
			desc.sceneMaterials->insert(sidMaterial, *itMaterial);

			auto sidName = vx::make_sid(actor.m_name);

			Actor a;
			a.m_mesh = sidMesh;
			a.m_material = sidMaterial;
			desc.sceneActors->insert(sidName, a);
		}
	}

	return true;
}

bool SceneFile::createSceneShared(const CreateSceneShared &desc)
{
	CreateSceneMeshInstancesDesc createMeshInstancesDesc;
	createMeshInstancesDesc.pMeshInstances = desc.pMeshInstances;
	createMeshInstancesDesc.sceneMaterials = desc.sceneMaterials;
	createMeshInstancesDesc.sceneMeshes = desc.sceneMeshes;
	createMeshInstancesDesc.sortedMaterials = desc.sortedMaterials;
	createMeshInstancesDesc.sortedMeshes = desc.sortedMeshes;
	createMeshInstancesDesc.sceneMeshInstanceNames = desc.sceneMeshInstanceNames;
	if (!createSceneMeshInstances(createMeshInstancesDesc))
		return false;

	CreateSceneActorsDesc createActorDesc;
	createActorDesc.sceneActors = desc.sceneActors;
	createActorDesc.sceneMaterials = desc.sceneMaterials;
	createActorDesc.sceneMeshes = desc.sceneMeshes;
	createActorDesc.sortedMaterials = desc.sortedMaterials;
	createActorDesc.sortedMeshes = desc.sortedMeshes;

	if (!createSceneActors(createActorDesc))
		return false;

	for (auto i = 0u; i < m_spawnCount; ++i)
	{
		desc.sceneSpawns[i].type = m_pSpawns[i].type;
		desc.sceneSpawns[i].position = m_pSpawns[i].position;
		desc.sceneSpawns[i].sid = vx::make_sid(m_pSpawns[i].actor);
	}

	*desc.vertexCount = 0;
	*desc.indexCount = 0u;
	for (auto &it : *desc.sceneMeshes)
	{
		auto &mesh = it->getMesh();
		*desc.vertexCount += mesh.getVertexCount();
		*desc.indexCount += mesh.getIndexCount();
	}

	return true;
}

u8 SceneFile::createScene(const CreateSceneDescription &desc)
{
	vx::sorted_vector<vx::StringID, Material*> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID, const vx::MeshFile*> sceneMeshes;
	vx::sorted_vector<vx::StringID, Actor> sceneActors;

	auto sceneSpawns = vx::make_unique<Spawn[]>(m_spawnCount);
	auto pMeshInstances = vx::make_unique<MeshInstance[]>(m_meshInstanceCount);

	u32 vertexCount = 0;
	u32 indexCount = 0;

	CreateSceneShared sharedDesc;
	sharedDesc.pMeshInstances = pMeshInstances.get();
	sharedDesc.sceneMaterials = &sceneMaterials;
	sharedDesc.sceneMeshes = &sceneMeshes;
	sharedDesc.sceneActors = &sceneActors;
	sharedDesc.sceneSpawns = sceneSpawns.get();
	sharedDesc.sortedMaterials = desc.sortedMaterials;;
	sharedDesc.sortedMeshes = desc.sortedMeshes;
	sharedDesc.vertexCount = &vertexCount;
	sharedDesc.indexCount = &indexCount;
	sharedDesc.sceneMeshInstanceNames = nullptr;
	if (!createSceneShared(sharedDesc))
		return 0;

	VX_ASSERT(desc.pScene);
	SceneParams sceneParams;
	sceneParams.m_baseParams.m_actors = std::move(sceneActors);
	sceneParams.m_baseParams.m_indexCount = indexCount;
	sceneParams.m_baseParams.m_lightCount = m_lightCount;
	sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
	sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
	sceneParams.m_baseParams.m_navMesh = std::move(m_navMesh);
#if _VX_EDITOR
	sceneParams.m_baseParams.m_pLights.reserve(m_lightCount);
	for(u32 i =0;i < m_lightCount; ++i)
	{
		sceneParams.m_baseParams.m_pLights.push_back(m_pLights[i]);
	}
#else
	sceneParams.m_baseParams.m_pLights = std::move(m_pLights);
#endif
	sceneParams.m_baseParams.m_pSpawns = std::move(sceneSpawns);
	sceneParams.m_baseParams.m_spawnCount = m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstanceCount = m_meshInstanceCount;
	sceneParams.m_pMeshInstances = std::move(pMeshInstances);
	sceneParams.m_waypointCount = 0;
	sceneParams.m_waypoints;

	*desc.pScene = Scene(sceneParams);
	desc.pScene->sortMeshInstances();

	return 1;
}

u8 SceneFile::createScene(const CreateEditorSceneDescription &desc)
{
	vx::sorted_vector<vx::StringID, Material*> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID, const vx::MeshFile*> sceneMeshes;
	vx::sorted_vector<vx::StringID, Actor> sceneActors;
	vx::sorted_vector<vx::StringID, std::string> sceneMeshInstanceNames;
	std::vector<MeshInstance> meshInstances(m_meshInstanceCount);
	auto sceneSpawns = vx::make_unique<Spawn[]>(m_spawnCount);

	u32 vertexCount = 0;
	u32 indexCount = 0;

	CreateSceneShared sharedDesc;
	sharedDesc.pMeshInstances = meshInstances.data();
	sharedDesc.sceneMaterials = &sceneMaterials;
	sharedDesc.sceneMeshes = &sceneMeshes;
	sharedDesc.sceneActors = &sceneActors;
	sharedDesc.sceneSpawns = sceneSpawns.get();
	sharedDesc.sortedMaterials = desc.sortedMaterials;
	sharedDesc.sortedMeshes = desc.sortedMeshes;
	sharedDesc.vertexCount = &vertexCount;
	sharedDesc.indexCount = &indexCount;
	sharedDesc.sceneMeshInstanceNames = &sceneMeshInstanceNames;
	if (!createSceneShared(sharedDesc))
		return 0;

	auto materialCount = sceneMaterials.size();
	vx::sorted_vector<vx::StringID, std::string> materialNames;
	materialNames.reserve(materialCount);
	for (u32 i = 0; i < materialCount; ++i)
	{
		auto sid = sceneMaterials.keys()[i];

		auto it = desc.loadedFiles->find(sid);

		//char str[32];
		//strncpy(str, it->c_str(), 32);

		materialNames.insert(sid, *it);
	}

	auto meshCount = sceneMeshes.size();
	vx::sorted_vector<vx::StringID, std::string> meshNames;
	meshNames.reserve(meshCount);
	for (u32 i = 0; i < meshCount; ++i)
	{
		auto sid = sceneMeshes.keys()[i];

		auto it = desc.loadedFiles->find(sid);

		//char str[32];
		//strncpy(str, it->c_str(), 32);

		meshNames.insert(sid, *it);
	}

	vx::sorted_vector<vx::StringID, std::string> actorNames;
	actorNames.reserve(m_actorCount);
	for (u32 i = 0; i < m_actorCount; ++i)
	{
		auto &actorFile = m_pActors[i];
		
		auto sid = vx::make_sid(actorFile.m_name);

		//char str[32];
	//	strncpy(str, actorFile.m_name, 32);

		actorNames.insert(sid, actorFile.m_name);
	}

	EditorSceneParams sceneParams;
	sceneParams.m_baseParams.m_actors = std::move(sceneActors);
	sceneParams.m_baseParams.m_indexCount = indexCount;
	sceneParams.m_baseParams.m_lightCount = m_lightCount;
	sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
	sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
	sceneParams.m_baseParams.m_navMesh = std::move(m_navMesh);
	sceneParams.m_meshInstanceNames = std::move(sceneMeshInstanceNames);
#if _VX_EDITOR
	sceneParams.m_baseParams.m_pLights.reserve(m_lightCount);
	for (u32 i = 0; i < m_lightCount; ++i)
	{
		sceneParams.m_baseParams.m_pLights.push_back(m_pLights[i]);
	}
#else
	sceneParams.m_baseParams.m_pLights = std::move(m_pLights);
#endif
	sceneParams.m_baseParams.m_pSpawns = std::move(sceneSpawns);
	sceneParams.m_baseParams.m_spawnCount = m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstances = std::move(meshInstances);
	sceneParams.m_waypoints;


	sceneParams.m_materialNames = std::move(materialNames);
	sceneParams.m_meshNames = std::move(meshNames);
	sceneParams.m_actorNames = std::move(actorNames);

	VX_ASSERT(desc.pScene);
	*desc.pScene = EditorScene(std::move(sceneParams));
	desc.pScene->sortMeshInstances();

	return 1;
}

u64 SceneFile::getCrc() const
{
	auto navMeshVertexSize = sizeof(vx::float3) * m_navMesh.getVertexCount();
	auto navMeshTriangleSize = sizeof(u16) * m_navMesh.getTriangleCount() * 3;
	auto navMeshSize = navMeshVertexSize + navMeshTriangleSize;

	auto meshInstanceSize = sizeof(MeshInstanceFile) * m_meshInstanceCount;
	auto lightSize = sizeof(Light) * m_lightCount;
	auto spawnSize = sizeof(SpawnFile) * m_spawnCount;
	auto actorSize = sizeof(ActorFile) * m_actorCount;

	auto totalSize = meshInstanceSize + lightSize + spawnSize + actorSize + navMeshSize;
	auto ptr = vx::make_unique<u8[]>(totalSize);

	auto offset = 0;
	::memcpy(ptr.get() + offset, m_pMeshInstances.get(), meshInstanceSize);
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
}

u32 SceneFile::getVersion() const
{
	return 1;
}