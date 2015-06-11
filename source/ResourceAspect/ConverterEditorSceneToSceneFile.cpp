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
#if _VX_EDITOR
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <vxResourceAspect/ConverterEditorSceneToSceneFile.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/NavMesh.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Waypoint.h>
#include <vxResourceAspect/SceneFile.h>

void ConverterEditorSceneToSceneFile::convert(const Editor::Scene &scene, SceneFile* sceneFile)
{
	auto actorCount = scene.m_actors.size();
	sceneFile->m_actorCount = actorCount;
	sceneFile->m_pActors = vx::make_unique<ActorFile[]>(actorCount);
	for (u32 i = 0; i < actorCount; ++i)
	{
		auto sidActor = scene.m_actors.keys()[i];
		auto &it = scene.m_actors[i];

		vx::StringID sidMaterial = it.m_material;
		vx::StringID sidMesh = it.m_mesh;

		auto actorNameIt = scene.m_actorNames.find(sidActor);
		auto meshNameIt = scene.m_meshNames.find(sidMesh);
		auto materialNameIt = scene.m_materialNames.find(sidMaterial);

		strncpy(sceneFile->m_pActors[i].m_material, materialNameIt->c_str(), 32);
		strncpy(sceneFile->m_pActors[i].m_mesh, meshNameIt->c_str(), 32);
		strncpy(sceneFile->m_pActors[i].m_name, actorNameIt->c_str(), 32);
	}

	sceneFile->m_lightCount = scene.m_lightCount;
	sceneFile->m_pLights = vx::make_unique<Light[]>(scene.m_lightCount);
	for (u32 i = 0; i < scene.m_lightCount; ++i)
	{
		sceneFile->m_pLights[i] = scene.m_pLights[i];
	}

	scene.m_navMesh.copy(&sceneFile->m_navMesh);

	auto meshInstanceCount = scene.m_meshInstances.size();

	sceneFile->m_meshInstanceCount = meshInstanceCount;
	sceneFile->m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(meshInstanceCount);
	for (u32 i = 0; i < meshInstanceCount; ++i)
	{
		auto &it = scene.m_meshInstances[i];

		auto instanceName = scene.getMeshInstanceName(it.getNameSid());
		auto instanceMeshName = scene.getMeshName(it.getMeshSid());
		auto instanceMaterialName = scene.getMaterialName(it.getMaterialSid());

		char meshName[32];
		strncpy(meshName, instanceMeshName, 32);

		char materialName[32];
		strncpy(materialName, instanceMaterialName, 32);

		char name[32];
		strncpy(name, instanceName, 32);

		auto transform = it.getTransform();
		sceneFile->m_pMeshInstances[i] = MeshInstanceFile(name, meshName, materialName, transform);
	}

	auto spawns = scene.getSpawns();
	auto spawnCount = scene.getSpawnCount();
	sceneFile->m_spawnCount = spawnCount;
	if (spawnCount != 0)
	{
		sceneFile->m_pSpawns = vx::make_unique<SpawnFile[]>(spawnCount);
		
		for (u32 i = 0; i < spawnCount; ++i)
		{
			auto &spawn = spawns[i];

			if (spawn.type != PlayerType::Human)
			{
				auto actorName = scene.getActorName(spawn.sid);
				strcpy(sceneFile->m_pSpawns[i].actor, actorName);
			}
			else
			{
				sceneFile->m_pSpawns[i].actor[0] = '\0';
			}
			
			sceneFile->m_pSpawns[i].position = spawn.position;
			sceneFile->m_pSpawns[i].type = spawn.type;
		}
	}

	sceneFile->m_waypointCount = scene.m_waypointCount;
	sceneFile->m_waypoints = vx::make_unique<Waypoint[]>(scene.m_waypointCount);
	for (u32 i = 0; i < scene.m_waypointCount; ++i)
	{
		sceneFile->m_waypoints[i] = scene.m_waypoints[i];
	}
}