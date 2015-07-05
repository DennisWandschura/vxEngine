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

struct SceneFile::CreateSceneMeshInstancesDesc
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>> *sortedMaterials;
	MeshInstance* pMeshInstances;
	vx::sorted_vector<vx::StringID, Editor::MeshInstance>* sortedMeshInstances;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
	vx::sorted_vector<vx::StringID, Reference<Material>>* sceneMaterials;
	vx::sorted_vector<vx::StringID, std::string>* sceneMeshInstanceNames;
};

struct SceneFile::CreateSceneActorsDesc
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>> *sortedMaterials;
	vx::sorted_vector<vx::StringID, Actor>* sceneActors;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
	vx::sorted_vector<vx::StringID, Reference<Material>>* sceneMaterials;
};

struct SceneFile::CreateSceneShared
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>> *sortedMaterials;
	MeshInstance* pMeshInstances;
	vx::sorted_vector<vx::StringID, Editor::MeshInstance>* sortedMeshInstances;
	vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
	vx::sorted_vector<vx::StringID, Reference<Material>>* sceneMaterials;
	vx::sorted_vector<vx::StringID, Actor>* sceneActors;
	vx::sorted_vector<vx::StringID, std::string>* sceneMeshInstanceNames;
	Spawn* sceneSpawns;
	u32* vertexCount;
	u32* indexCount;
};

SceneFile::SceneFile(u32 version)
	:Serializable(version),
	m_pMeshInstances(),
	m_pLights(),
	m_pSpawns(),
	m_pActors(),
	m_waypoints(),
	m_navMesh(),
	m_meshInstanceCount(0),
	m_lightCount(0),
	m_spawnCount(0),
	m_actorCount(0),
	m_waypointCount(0)
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
		m_navMesh.swap(other.m_navMesh);
		std::swap(m_meshInstanceCount, other.m_meshInstanceCount);
		std::swap(m_lightCount, other.m_lightCount);
		std::swap(m_spawnCount, other.m_spawnCount);
		std::swap(m_actorCount, other.m_actorCount);
		std::swap(m_waypointCount, other.m_waypointCount);
	}
}

const u8* SceneFile::loadVersion3(const u8 *ptr, const u8* last, vx::Allocator* allocator)
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
}

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

	ptr = vx::read(m_pMeshInstances.get(), ptr, m_meshInstanceCount);
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
	if (version == 3)
	{
		result = loadVersion3(ptr, last, allocator);
	}
	else if (version == 4)
	{
		result = loadVersion4(ptr, last, allocator);
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

	file->write(m_pMeshInstances.get(), m_meshInstanceCount);
	file->write(m_pLights.get(), m_lightCount);
	file->write(m_pSpawns.get(), m_spawnCount);
	file->write(m_pActors.get(), m_actorCount);
	file->write(m_waypoints.get(), m_waypointCount);

	m_navMesh.saveToFile(file);
}

const MeshInstanceFile* SceneFile::getMeshInstances() const noexcept
{
	return m_pMeshInstances.get();
}

u32 SceneFile::getNumMeshInstances() const noexcept
{
	return m_meshInstanceCount;
}

bool SceneFile::createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc)
{
	struct InstanceInserter
	{
		typedef void(InstanceInserter::*InserterFun)(const MeshInstance &instance, u32 i, const char* name);

		InserterFun m_fp;
		MeshInstance* m_instances;
		vx::sorted_vector<vx::StringID, Editor::MeshInstance>* m_sortedInstances;

		InstanceInserter(const CreateSceneMeshInstancesDesc &desc)
			:m_instances(desc.pMeshInstances),
			m_sortedInstances(desc.sortedMeshInstances)
		{
			if (m_instances == nullptr)
			{
				m_fp = insertSorted;
			}
			else
			{
				m_fp = insertArray;
			}
		}

		void insertArray(const MeshInstance &instance, u32 i, const char*)
		{
			m_instances[i] = instance;
		}

		void insertSorted(const MeshInstance &instance, u32, const char* name)
		{
			Editor::MeshInstance editorInstance(instance, std::string(name));
			m_sortedInstances->insert(instance.getNameSid(), editorInstance);
		}

		void operator()(const MeshInstance &instance, u32 i, const char* name)
		{
			(this->*m_fp)(instance, i, name);
		}
	};

	InstanceInserter instanceInserter(desc);

	for (auto i = 0u; i < m_meshInstanceCount; ++i)
	{
			std::string genName = "instance" + std::to_string(i);

			auto &instanceFile = m_pMeshInstances[i];
			auto name = instanceFile.getName();
			auto meshFile = instanceFile.getMeshFile();
			auto materialFile = instanceFile.getMaterialFile();

			if (name[0] == '\0')
				name = genName.c_str();

			auto sidName = vx::FileHandle(name);
			auto sidMesh = vx::FileHandle(meshFile);
			auto sidAnimation = vx::FileHandle(instanceFile.getAnimation());
			auto itMesh = desc.sortedMeshes->find(sidMesh.m_sid);
			auto sidMaterial = vx::FileHandle(materialFile);
			auto itMaterial = desc.sortedMaterials->find(sidMaterial.m_sid);

			if (itMesh == desc.sortedMeshes->end() || itMaterial == desc.sortedMaterials->end())
			{
				return false;
			}

			desc.sceneMeshes->insert(sidMesh.m_sid, *itMesh);
			desc.sceneMaterials->insert(sidMaterial.m_sid, *itMaterial);

			auto transform = instanceFile.getTransform();

			auto instance = MeshInstance(sidName.m_sid, sidMesh.m_sid, *itMaterial, sidAnimation.m_sid, transform);
			instanceInserter(instance, i, name);

			if (desc.sceneMeshInstanceNames)
			{
				desc.sceneMeshInstanceNames->insert(sidName.m_sid, std::string(name));
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

			auto sidMesh = vx::FileHandle(actor.m_mesh);
			auto itMesh = desc.sortedMeshes->find(sidMesh.m_sid);

			auto sidMaterial = vx::FileHandle(actor.m_material);
			auto itMaterial = desc.sortedMaterials->find(sidMaterial.m_sid);

			if (itMesh == desc.sortedMeshes->end() || itMaterial == desc.sortedMaterials->end())
			{
				return false;
			}

			desc.sceneMeshes->insert(sidMesh.m_sid, *itMesh);
			desc.sceneMaterials->insert(sidMaterial.m_sid, *itMaterial);

			auto sidName = vx::make_sid(actor.m_name);

			Actor a;
			a.m_mesh = sidMesh.m_sid;
			a.m_material = sidMaterial.m_sid;
			desc.sceneActors->insert(sidName, a);
		}
	}

	return true;
}

bool SceneFile::createSceneShared(const CreateSceneShared &desc)
{
	CreateSceneMeshInstancesDesc createMeshInstancesDesc;
	createMeshInstancesDesc.pMeshInstances = desc.pMeshInstances;
	createMeshInstancesDesc.sortedMeshInstances = desc.sortedMeshInstances;
	createMeshInstancesDesc.sceneMaterials = desc.sceneMaterials;
	createMeshInstancesDesc.sceneMeshes = desc.sceneMeshes;
	createMeshInstancesDesc.sortedMaterials = desc.sortedMaterials;
	createMeshInstancesDesc.sortedMeshes = desc.sortedMeshes;
	createMeshInstancesDesc.sceneMeshInstanceNames = desc.sceneMeshInstanceNames;
	if (!createSceneMeshInstances(createMeshInstancesDesc))
	{
		printf("SceneFile::createSceneShared::createSceneMeshInstances error\n");
		return false;
	}

	CreateSceneActorsDesc createActorDesc;
	createActorDesc.sceneActors = desc.sceneActors;
	createActorDesc.sceneMaterials = desc.sceneMaterials;
	createActorDesc.sceneMeshes = desc.sceneMeshes;
	createActorDesc.sortedMaterials = desc.sortedMaterials;
	createActorDesc.sortedMeshes = desc.sortedMeshes;

	if (!createSceneActors(createActorDesc))
	{
		printf("SceneFile::createSceneShared::createSceneActors error\n");
		return false;
	}

	for (auto i = 0u; i < m_spawnCount; ++i)
	{
		desc.sceneSpawns[i].type = m_pSpawns[i].type;
		desc.sceneSpawns[i].position = m_pSpawns[i].position;
		desc.sceneSpawns[i].sid = vx::FileHandle(m_pSpawns[i].actor).m_sid;
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
	vx::sorted_vector<vx::StringID, Reference<Material>> sceneMaterials;
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

	sceneParams.m_baseParams.m_pLights.reserve(m_lightCount);
	for(u32 i =0;i < m_lightCount; ++i)
	{
		sceneParams.m_baseParams.m_pLights.push_back(m_pLights[i]);
	}

	sceneParams.m_baseParams.m_pSpawns.reserve(m_spawnCount);
	for (u32 i = 0; i < m_spawnCount; ++i)
	{
		sceneParams.m_baseParams.m_pSpawns.insert(std::move(sceneSpawns[i].id), std::move(sceneSpawns[i]));
	}

	sceneParams.m_baseParams.m_spawnCount = m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstanceCount = m_meshInstanceCount;
	sceneParams.m_pMeshInstances = std::move(pMeshInstances);
	sceneParams.m_baseParams.m_waypointCount = m_waypointCount;

	sceneParams.m_baseParams.m_waypoints.reserve(m_waypointCount);
	for(u32 i =0;i < m_waypointCount; ++i)
	{
		sceneParams.m_baseParams.m_waypoints.push_back(m_waypoints[i]);
	}

	*desc.pScene = Scene(sceneParams);
	desc.pScene->sortMeshInstances();

	return 1;
}

u8 SceneFile::createScene(const CreateEditorSceneDescription &desc)
{
	vx::sorted_vector<vx::StringID, Reference<Material>> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID, const vx::MeshFile*> sceneMeshes;
	vx::sorted_vector<vx::StringID, Actor> sceneActors;
	vx::sorted_vector<vx::StringID, std::string> sceneMeshInstanceNames;
	vx::sorted_vector<vx::StringID, Editor::MeshInstance> meshInstances;
	meshInstances.reserve(m_meshInstanceCount);
	auto sceneSpawns = vx::make_unique<Spawn[]>(m_spawnCount);

	u32 vertexCount = 0;
	u32 indexCount = 0;

	CreateSceneShared sharedDesc;
	sharedDesc.pMeshInstances = nullptr;
	sharedDesc.sortedMeshInstances = &meshInstances;
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
	{
		printf("SceneFile::createScene error\n");
		return 0;
	}

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
		
		auto sid = vx::FileHandle(actorFile.m_name);

		//char str[32];
	//	strncpy(str, actorFile.m_name, 32);

		actorNames.insert(sid.m_sid, actorFile.m_name);
	}

	Editor::SceneParams sceneParams;
	sceneParams.m_baseParams.m_actors = std::move(sceneActors);
	sceneParams.m_baseParams.m_indexCount = indexCount;
	sceneParams.m_baseParams.m_lightCount = m_lightCount;
	sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
	sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
	sceneParams.m_baseParams.m_navMesh = std::move(m_navMesh);
	sceneParams.m_meshInstanceNames = std::move(sceneMeshInstanceNames);

	sceneParams.m_baseParams.m_pLights.reserve(m_lightCount);
	for (u32 i = 0; i < m_lightCount; ++i)
	{
		sceneParams.m_baseParams.m_pLights.push_back(m_pLights[i]);
	}

	sceneParams.m_baseParams.m_pSpawns.reserve(m_spawnCount);
	for (u32 i = 0; i < m_lightCount; ++i)
	{
		sceneParams.m_baseParams.m_pSpawns.insert(std::move(sceneSpawns[i].id), std::move(sceneSpawns[i]));
	}

	sceneParams.m_baseParams.m_spawnCount = m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstances = std::move(meshInstances);

	sceneParams.m_baseParams.m_waypointCount = m_waypointCount;

	std::vector<Waypoint> waypointsEditor;
	waypointsEditor.reserve(m_waypointCount);
	for (u32 i = 0; i < m_waypointCount; ++i)
	{
		waypointsEditor.push_back(m_waypoints[i]);
	}
	sceneParams.m_baseParams.m_waypoints = std::move(waypointsEditor);

	sceneParams.m_materialNames = std::move(materialNames);
	sceneParams.m_meshNames = std::move(meshNames);
	sceneParams.m_actorNames = std::move(actorNames);

	VX_ASSERT(desc.pScene);
	*desc.pScene = Editor::Scene(std::move(sceneParams));
	desc.pScene->sortMeshInstances();

	return 1;
}

u64 SceneFile::getCrcVersion3() const
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
}

u64 SceneFile::getCrcVersion4() const
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
	if (currentVersion == 3)
		crc = getCrcVersion3();
	else if (currentVersion == 4)
		crc = getCrcVersion4();
	
	return crc;
}

u32 SceneFile::getGlobalVersion()
{
	return 4;
}