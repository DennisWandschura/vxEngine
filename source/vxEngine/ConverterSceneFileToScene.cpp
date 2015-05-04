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

bool ConverterSceneFileToScene::createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc)
{
	for (auto i = 0u; i < desc.sceneFile->m_meshInstanceCount; ++i)
	{
		auto &instance = desc.sceneFile->m_pMeshInstances[i];
		auto meshFile = instance.getMeshFile();
		auto materialFile = instance.getMaterialFile();

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

		desc.pMeshInstances[i] = MeshInstance(sidMesh, sidMaterial, instance.getTransform());
	}

	return true;
}

bool ConverterSceneFileToScene::createSceneActors(const CreateSceneActorsDesc &desc)
{
	auto actorCount = desc.sceneFile->m_actorCount;
	if (actorCount != 0)
	{
		desc.sceneActors->reserve(actorCount);
		for (auto i = 0u; i < actorCount; ++i)
		{
			auto &actor = desc.sceneFile->m_pActors[i];

			auto sidMesh = vx::make_sid(actor.mesh);
			auto itMesh = desc.sortedMeshes->find(sidMesh);

			auto sidMaterial = vx::make_sid(actor.material);
			auto itMaterial = desc.sortedMaterials->find(sidMaterial);

			if (itMesh == desc.sortedMeshes->end() || itMaterial == desc.sortedMaterials->end())
			{
				return false;
			}

			desc.sceneMeshes->insert(sidMesh, *itMesh);
			desc.sceneMaterials->insert(sidMaterial, *itMaterial);

			auto sidName = vx::make_sid(actor.name);

			Actor a;
			a.mesh = sidMesh;
			a.material = sidMaterial;
			desc.sceneActors->insert(sidName, a);
		}
	}

	return true;
}

bool ConverterSceneFileToScene::convert(const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes, const vx::sorted_array<vx::StringID, Material*> *sortedMaterials, const SceneFile &sceneFile, Scene* scene)
{
	vx::sorted_vector<vx::StringID, Material*> sceneMaterials;
	sceneMaterials.reserve(5);

	vx::sorted_vector<vx::StringID, const vx::Mesh*> sceneMeshes;

	auto pMeshInstances = std::make_unique<MeshInstance[]>(sceneFile.m_meshInstanceCount);

	CreateSceneMeshInstancesDesc desc;
	desc.pMeshInstances = pMeshInstances.get();
	desc.sceneFile = &sceneFile;
	desc.sceneMaterials = &sceneMaterials;
	desc.sceneMeshes = &sceneMeshes;
	desc.sortedMaterials = sortedMaterials;
	desc.sortedMeshes = sortedMeshes;

	if (!createSceneMeshInstances(desc))
		return 0;

	vx::sorted_vector<vx::StringID, Actor> sceneActors;

	CreateSceneActorsDesc createSceneActorsDesc;
	createSceneActorsDesc.sceneActors = &sceneActors;
	createSceneActorsDesc.sceneFile = &sceneFile;
	createSceneActorsDesc.sceneMaterials = &sceneMaterials;
	createSceneActorsDesc.sceneMeshes = &sceneMeshes;
	createSceneActorsDesc.sortedMaterials = sortedMaterials;
	createSceneActorsDesc.sortedMeshes = sortedMeshes;
	if (!createSceneActors(createSceneActorsDesc))
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
	sceneParams.m_baseParams.m_actors = std::move(sceneActors);
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