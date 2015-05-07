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
#include "ConverterEditorSceneToSceneFile.h"
#include "EditorScene.h"
#include "SceneFile.h"
#include "Actor.h"
#include "Light.h"
#include "NavMesh.h"
#include "MeshInstance.h"
#include "Spawn.h"
#include "enums.h"

void ConverterEditorSceneToSceneFile::convert(const EditorScene &scene, SceneFile* sceneFile)
{
	auto actorCount = scene.m_actors.size();
	sceneFile->m_actorCount = actorCount;
	sceneFile->m_pActors = std::make_unique<ActorFile[]>(actorCount);
	for (u32 i = 0; i < actorCount; ++i)
	{
		auto sidActor = scene.m_actors.keys()[i];
		auto &it = scene.m_actors[i];

		vx::StringID sidMaterial = it.m_material;
		vx::StringID sidMesh = it.m_mesh;

		auto actorNameIt = scene.m_actorNames.find(sidActor);
		auto meshNameIt = scene.m_meshNames.find(sidMesh);
		auto materialNameIt = scene.m_materialNames.find(sidMaterial);

		strncpy_s(sceneFile->m_pActors[i].m_material, *materialNameIt, 32);
		strncpy_s(sceneFile->m_pActors[i].m_mesh, *meshNameIt, 32);
		strncpy_s(sceneFile->m_pActors[i].m_name, *actorNameIt, 32);
	}

	sceneFile->m_lightCount = scene.m_lightCount;
	sceneFile->m_pLights = std::make_unique<Light[]>(scene.m_lightCount);
	for (u32 i = 0; i < scene.m_lightCount; ++i)
	{
		sceneFile->m_pLights[i] = scene.m_pLights[i];
	}

	scene.m_navMesh.copyTo(&sceneFile->m_navMesh);


	auto meshInstanceCount = scene.m_meshInstances.size();

	sceneFile->m_meshInstanceCount = meshInstanceCount;
	sceneFile->m_pMeshInstances = std::make_unique<MeshInstanceFile[]>(meshInstanceCount);
	for (u32 i = 0; i < meshInstanceCount; ++i)
	{
		auto &it = scene.m_meshInstances[i];

		auto instanceMeshName = scene.getMeshName(it.getMeshSid());
		auto instanceMaterialName = scene.getMaterialName(it.getMaterialSid());

		char meshName[32];
		strcpy_s(meshName, instanceMeshName);

		char materialName[32];
		strcpy_s(materialName, instanceMaterialName);

		sceneFile->m_pMeshInstances[i] = MeshInstanceFile(meshName, materialName,it.getTransform());
	}

	auto spawns = scene.getSpawns();
	auto spawnCount = scene.getSpawnCount();
	sceneFile->m_spawnCount = spawnCount;
	if (spawnCount != 0)
	{
		sceneFile->m_pSpawns = std::make_unique<SpawnFile[]>(spawnCount);
		
		for (u32 i = 0; i < spawnCount; ++i)
		{
			auto &spawn = spawns[i];

			if (spawn.type != PlayerType::Human)
			{
				auto actorName = scene.getActorName(spawn.sid);
				strcpy_s(sceneFile->m_pSpawns[i].actor, actorName);
			}
			else
			{
				sceneFile->m_pSpawns[i].actor[0] = '\0';
			}
			
			sceneFile->m_pSpawns[i].position = spawn.position;
			sceneFile->m_pSpawns[i].type = spawn.type;
		}
	}
}