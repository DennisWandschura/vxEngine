#include "ConverterSceneFileToScene.h"
#include "MeshInstance.h"
#include <memory>
#include "Scene.h"
#include "SceneFile.h"
#include "Actor.h"
#include "Spawn.h"
#include <vxLib/Graphics/Mesh.h>
#include "Light.h"
#include "Waypoint.h"
#include "Material.h"

bool ConverterSceneFileToScene::createSceneMeshInstances(const SceneFile &sceneFile, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
	MeshInstance* pMeshInstances, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials)
{
	for (auto i = 0u; i < sceneFile.m_meshInstanceCount; ++i)
	{
		auto &instance = sceneFile.m_pMeshInstances[i];
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

bool ConverterSceneFileToScene::createSceneActors(const SceneFile &sceneFile, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
	vx::sorted_vector<vx::StringID64, Actor>* actors, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials)
{
	if (sceneFile.m_actorCount != 0)
	{
		actors->reserve(sceneFile.m_actorCount);
		for (auto i = 0u; i < sceneFile.m_actorCount; ++i)
		{
			auto &actor = sceneFile.m_pActors[i];

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

bool ConverterSceneFileToScene::convert(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials, const SceneFile &sceneFile, Scene* scene)
{
	vx::sorted_vector<vx::StringID64, Material*> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID64, const vx::Mesh*> sceneMeshes;

	auto pMeshInstances = std::make_unique<MeshInstance[]>(sceneFile.m_meshInstanceCount);
	if (!createSceneMeshInstances(sceneFile, meshes, materials, pMeshInstances.get(), &sceneMeshes, &sceneMaterials))
		return 0;

	vx::sorted_vector<vx::StringID64, Actor> actors;
	if (!createSceneActors(sceneFile, meshes, materials, &actors, &sceneMeshes, &sceneMaterials))
		return 0;

	auto spawns = std::make_unique<Spawn[]>(sceneFile.m_spawnCount);
	for (auto i = 0u; i < sceneFile.m_spawnCount; ++i)
	{
		spawns[i].type = sceneFile.m_pSpawns[i].type;
		spawns[i].position = sceneFile.m_pSpawns[i].position;
		spawns[i].sid = vx::make_sid(sceneFile.m_pSpawns[i].actor);
	}

	U32 vertexCount = 0;
	auto indexCount = 0u;
	for (auto &it : sceneMeshes)
	{
		vertexCount += it->getVertexCount();
		indexCount += it->getIndexCount();
	}

	NavMesh navMesh;
	sceneFile.m_navMesh.copyTo(&navMesh);

	auto pLights = std::make_unique<Light[]>(sceneFile.m_lightCount);
	for (U32 i = 0; i < sceneFile.m_lightCount; ++i)
	{
		pLights[i] = sceneFile.m_pLights[i];
	}

	SceneParams sceneParams;
	sceneParams.m_baseParams.m_actors = std::move(actors);
	sceneParams.m_baseParams.m_indexCount = indexCount;
	sceneParams.m_baseParams.m_lightCount = sceneFile.m_lightCount;
	sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
	sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
	sceneParams.m_baseParams.m_navMesh = std::move(navMesh);
	sceneParams.m_baseParams.m_pLights = std::move(pLights);
	sceneParams.m_baseParams.m_pSpawns = std::move(spawns);
	sceneParams.m_baseParams.m_spawnCount = sceneFile.m_spawnCount;
	sceneParams.m_baseParams.m_vertexCount = vertexCount;
	sceneParams.m_meshInstanceCount = sceneFile.m_meshInstanceCount;
	sceneParams.m_pMeshInstances = std::move(pMeshInstances);
	sceneParams.m_waypointCount = 0;
	sceneParams.m_waypoints;

	*scene = Scene(sceneParams);
	scene->sortMeshInstances();

	return 1;
}