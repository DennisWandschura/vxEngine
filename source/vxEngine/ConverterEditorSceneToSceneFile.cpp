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
	for (U32 i = 0; i < actorCount; ++i)
	{
		auto sidActor = scene.m_actors.keys()[i];
		auto &it = scene.m_actors[i];

		vx::StringID sidMaterial = it.material;
		vx::StringID sidMesh = it.mesh;

		auto actorNameIt = scene.m_actorNames.find(sidActor);
		auto meshNameIt = scene.m_meshNames.find(sidMesh);
		auto materialNameIt = scene.m_materialNames.find(sidMaterial);

		strncpy_s(sceneFile->m_pActors[i].material, *materialNameIt, 32);
		strncpy_s(sceneFile->m_pActors[i].mesh, *meshNameIt, 32);
		strncpy_s(sceneFile->m_pActors[i].name, *actorNameIt, 32);
	}

	sceneFile->m_lightCount = scene.m_lightCount;
	sceneFile->m_pLights = std::make_unique<Light[]>(scene.m_lightCount);
	for (U32 i = 0; i < scene.m_lightCount; ++i)
	{
		sceneFile->m_pLights[i] = scene.m_pLights[i];
	}

	scene.m_navMesh.copyTo(&sceneFile->m_navMesh);


	auto meshInstanceCount = scene.m_meshInstances.size();

	sceneFile->m_meshInstanceCount = meshInstanceCount;
	sceneFile->m_pMeshInstances = std::make_unique<MeshInstanceFile[]>(meshInstanceCount);
	for (U32 i = 0; i < meshInstanceCount; ++i)
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
		
		for (U32 i = 0; i < spawnCount; ++i)
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