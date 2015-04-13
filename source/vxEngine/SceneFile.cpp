#include "SceneFile.h"
#include "utility.h"
#include "yamlHelper.h"
#include "File.h"
#include "MeshInstance.h"
#include "Material.h"
#include <fstream>
#include <vxLib/Container/sorted_array.h>
#include "Actor.h"
#include "Spawn.h"
#include <vxLib/Graphics/Mesh.h>
#include "EditorScene.h"
#include "Light.h"
#include "Scene.h"
#include "Waypoint.h"

namespace YAML
{
	template<>
	struct convert < MeshInstanceFile >
	{
		static bool decode(const YAML::Node &node, MeshInstanceFile &data)
		{
			data.load(node);
			return true;
		}

		static Node encode(const MeshInstanceFile &rhs)
		{
			Node n;
			rhs.save(n);
			return n;
		}
	};

	template<>
	struct convert < ActorFile >
	{
		static bool decode(const YAML::Node &node, ActorFile &data)
		{
			std::string strMesh = node["mesh"].as<std::string>();
			std::string strMaterial = node["material"].as<std::string>();
			std::string strName = node["name"].as<std::string>();

			strncpy_s(data.mesh, strMesh.data(), strMesh.size());
			strncpy_s(data.material, strMaterial.data(), strMaterial.size());
			strncpy_s(data.name, strName.data(), strName.size());

			return true;
		}

		static Node encode(const ActorFile &rhs)
		{
			Node n;

			n["mesh"] = std::string(rhs.mesh);
			n["material"] = std::string(rhs.material);
			n["name"] = std::string(rhs.name);

			return n;
		}
	};
}

SceneFile::SceneFile()
{
}

SceneFile::~SceneFile()
{
}

const U8* SceneFile::load(const U8 *ptr)
{
	ptr = vx::read(m_meshInstanceCount, ptr);
	ptr = vx::read(m_lightCount, ptr);
	ptr = vx::read(m_spawnCount, ptr);
	ptr = vx::read(m_actorCount, ptr);

	m_pMeshInstances = std::make_unique<MeshInstanceFile[]>(m_meshInstanceCount);
	m_pLights = std::make_unique<Light[]>(m_lightCount);
	m_pSpawns = std::make_unique<SpawnFile[]>(m_spawnCount);

	ptr = vx::read(m_pMeshInstances.get(), ptr, m_meshInstanceCount);
	ptr = vx::read(m_pLights.get(), ptr, m_lightCount);
	ptr = vx::read(m_pSpawns.get(), ptr, m_spawnCount);

	if (m_actorCount != 0)
	{
		m_pActors = std::make_unique<ActorFile[]>(m_actorCount);

		ptr = vx::read(m_pActors.get(), ptr, m_actorCount);
	}

	return m_navMesh.load(ptr);
}

void SceneFile::loadFromYAML(const char *file)
{
	auto root = YAML::LoadFile(file);

	auto spawns = SpawnFile::loadFromYaml(root["spawns"]);
	auto meshInstances = root["meshInstances"].as<std::vector<MeshInstanceFile>>();
	auto lights = Light::loadFromYaml(root["lights"]);
	auto actors = root["actors"].as<std::vector<ActorFile>>();
	m_navMesh.loadFromYAML(root["navmesh"]);

	m_pMeshInstances = std::make_unique<MeshInstanceFile[]>(meshInstances.size());
	vx::read(m_pMeshInstances.get(), (U8*)meshInstances.data(), meshInstances.size());

	m_pLights = std::make_unique<Light[]>(lights.size());
	vx::read(m_pLights.get(), (U8*)lights.data(), lights.size());

	m_pSpawns = std::make_unique<SpawnFile[]>(spawns.size());
	vx::read(m_pSpawns.get(), (U8*)spawns.data(), spawns.size());

	if (actors.size() != 0)
	{
		m_pActors = std::make_unique<ActorFile[]>(actors.size());
		vx::read(m_pActors.get(), (U8*)actors.data(), actors.size());
	}

	m_meshInstanceCount = meshInstances.size();
	m_lightCount = lights.size();
	m_spawnCount = spawns.size();
	m_actorCount = actors.size();
}

void SceneFile::saveToFile(const char *file) const
{
	File f;
	VX_ASSERT(f.create(file, FileAccess::Write));

	f.write(m_meshInstanceCount);
	f.write(m_lightCount);
	f.write(m_spawnCount);
	f.write(m_actorCount);

	f.write(m_pMeshInstances.get(), m_meshInstanceCount);
	f.write(m_pLights.get(), m_lightCount);
	f.write(m_pSpawns.get(), m_spawnCount);
	f.write(m_pActors.get(), m_actorCount);

	m_navMesh.saveToFile(&f);

	f.close();
}

void SceneFile::saveToFile(File *file) const
{
	file->write(m_meshInstanceCount);
	file->write(m_lightCount);
	file->write(m_spawnCount);
	file->write(m_actorCount);

	file->write(m_pMeshInstances.get(), m_meshInstanceCount);
	file->write(m_pLights.get(), m_lightCount);
	file->write(m_pSpawns.get(), m_spawnCount);
	file->write(m_pActors.get(), m_actorCount);

	m_navMesh.saveToFile(file);
}

void SceneFile::saveToYAML(const char *file) const
{
	YAML::Node meshNode;

	for (auto i = 0u; i < m_meshInstanceCount; ++i)
	{
		meshNode[i] = m_pMeshInstances[i];
	}

	YAML::Node lightNode = Light::saveToYaml(m_pLights.get(), m_lightCount);

	YAML::Node spawnsNode = SpawnFile::saveToYaml(m_pSpawns.get(), m_spawnCount);

	YAML::Node actorNode;
	for (auto i = 0u; i < m_actorCount; ++i)
	{
		actorNode[i] = m_pActors[i];
	}

	YAML::Node root;
	root["spawns"] = spawnsNode;
	root["meshInstances"] = meshNode;
	root["lights"] = lightNode;
	root["actors"] = actorNode;

	std::ofstream outfile(file);
	outfile << root;
}

const std::unique_ptr<MeshInstanceFile[]>& SceneFile::getMeshInstances() const noexcept
{
	return m_pMeshInstances;
}

U32 SceneFile::getNumMeshInstances() const noexcept
{
	return m_meshInstanceCount;
}

bool SceneFile::createSceneMeshInstances(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
MeshInstance* pMeshInstances, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials)
{
	for (auto i = 0u; i < m_meshInstanceCount; ++i)
	{
		auto &instance = m_pMeshInstances[i];
		auto meshFile = instance.getMeshFile();
		auto materialFile = instance.getMaterialFile();

		auto sidMesh = vx::make_sid(meshFile);
		auto itMesh = meshes.find(sidMesh);
		auto sidMaterial = vx::make_sid(materialFile);
		auto itMaterial = materials.find(sidMaterial);

		if (itMesh == meshes.end() || itMaterial == materials.end())
		{
			return false;
		}

		sceneMeshes->insert(sidMesh, &*itMesh);
		sceneMaterials->insert(sidMaterial, &*itMaterial);

		pMeshInstances[i] = MeshInstance(sidMesh, sidMaterial, instance.getTransform());
	}

	return true;
}

bool SceneFile::createSceneActors(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
	vx::sorted_vector<vx::StringID64, Actor>* actors, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials)
{
	if (m_actorCount != 0)
	{
		//pActors = std::make_unique<Actor[]>(m_actorCount);
		actors->reserve(m_actorCount);
		for (auto i = 0u; i < m_actorCount; ++i)
		{
			auto &actor = m_pActors[i];

			auto sidMesh = vx::make_sid(actor.mesh);
			auto itMesh = meshes.find(sidMesh);

			auto sidMaterial = vx::make_sid(actor.material);
			auto itMaterial = materials.find(sidMaterial);

			if (itMesh == meshes.end() || itMaterial == materials.end())
			{
				return false;
			}

			sceneMeshes->insert(sidMesh, &*itMesh);
			sceneMaterials->insert(sidMaterial, &*itMaterial);

			auto sidName = vx::make_sid(actor.name);

			Actor a;
			a.mesh = sidMesh;
			a.material = sidMaterial;
			actors->insert(sidName, a);
		}
	}

	return true;
}

U8 SceneFile::createScene(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials, Scene *pScene)
{
	vx::sorted_vector<vx::StringID64, Material*> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID64, const vx::Mesh*> sceneMeshes;

	auto pMeshInstances = std::make_unique<MeshInstance[]>(m_meshInstanceCount);
	if (!createSceneMeshInstances(meshes, materials, pMeshInstances.get(), &sceneMeshes, &sceneMaterials))
		return 0;

	vx::sorted_vector<vx::StringID64, Actor> actors;
	if (!createSceneActors(meshes, materials, &actors, &sceneMeshes, &sceneMaterials))
		return 0;

	auto spawns = std::make_unique<Spawn[]>(m_spawnCount);
	for (auto i = 0u; i < m_spawnCount; ++i)
	{
		spawns[i].type = m_pSpawns[i].type;
		spawns[i].position = m_pSpawns[i].position;
		spawns[i].sid = vx::make_sid(m_pSpawns[i].actor);
	}

	U32 vertexCount = 0;
	auto indexCount = 0u;
	for (auto &it : sceneMeshes)
	{
		vertexCount += it->getVertexCount();
		indexCount += it->getIndexCount();
	}

	VX_ASSERT(pScene);
	SceneParams sceneParams;
	sceneParams.m_baseParams.m_actors = std::move(actors);
	sceneParams.m_baseParams.m_indexCount = indexCount;
	sceneParams.m_baseParams.m_lightCount = m_lightCount;
	sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
	sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
	sceneParams.m_baseParams.m_navMesh = std::move(m_navMesh);
	sceneParams.m_baseParams.m_pLights = std::move(m_pLights);
	sceneParams.m_baseParams.m_pSpawns = std::move(spawns);
	sceneParams.m_baseParams.m_spawnCount = m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstanceCount = m_meshInstanceCount;
	sceneParams.m_pMeshInstances = std::move(pMeshInstances);
	sceneParams.m_waypointCount = 0;
	sceneParams.m_waypoints;

	*pScene = Scene(sceneParams);
	pScene->sortMeshInstances();

	return 1;
}

U8 SceneFile::createScene(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials, EditorScene *pScene)
{
	vx::sorted_vector<vx::StringID64, Material*> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID64, const vx::Mesh*> sceneMeshes;

	std::vector<MeshInstance> meshInstances(m_meshInstanceCount);
	//meshInstances.reserve(m_meshInstanceCount);
	if (createSceneMeshInstances(meshes, materials, meshInstances.data(), &sceneMeshes, &sceneMaterials) == 0)
		return 0;

	vx::sorted_vector<vx::StringID64, Actor> actors;
	if (createSceneActors(meshes, materials, &actors, &sceneMeshes, &sceneMaterials) == 0)
		return 0;

	auto spawns = std::make_unique<Spawn[]>(m_spawnCount);
	for (auto i = 0u; i < m_spawnCount; ++i)
	{
		spawns[i].type = m_pSpawns[i].type;
		spawns[i].position = m_pSpawns[i].position;
		spawns[i].sid = vx::make_sid(m_pSpawns[i].actor);
	}

	U32 vertexCount = 0;
	auto indexCount = 0u;
	for (auto &it : sceneMeshes)
	{
		vertexCount += it->getVertexCount();
		indexCount += it->getIndexCount();
	}

	EditorSceneParams sceneParams;
	sceneParams.m_baseParams.m_actors = std::move(actors);
	sceneParams.m_baseParams.m_indexCount = indexCount;
	sceneParams.m_baseParams.m_lightCount = m_lightCount;
	sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
	sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
	sceneParams.m_baseParams.m_navMesh = std::move(m_navMesh);
	sceneParams.m_baseParams.m_pLights = std::move(m_pLights);
	sceneParams.m_baseParams.m_pSpawns = std::move(spawns);
	sceneParams.m_baseParams.m_spawnCount = m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstances = std::move(meshInstances);
	sceneParams.m_waypoints;

	VX_ASSERT(pScene);
	*pScene = EditorScene(sceneParams);
	pScene->sortMeshInstances();

	return 1;
}