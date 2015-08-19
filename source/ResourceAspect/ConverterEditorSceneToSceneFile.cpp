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

#include <vxResourceAspect/ConverterEditorSceneToSceneFile.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/NavMesh.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Joint.h>

namespace Converter
{
	void EditorSceneToSceneFile::convertActors(const Editor::Scene &scene, SceneFile* sceneFile)
	{
		auto &sceneActors = scene.m_actors;
		auto actorCount = sceneActors.size();
		auto keys = sceneActors.keys();
		auto actors = sceneActors.data();

		if (actorCount != 0)
		{
			sceneFile->m_pActors = vx::make_unique<ActorFile[]>(actorCount);
			for (u32 i = 0; i < actorCount; ++i)
			{
				auto sidActor = keys[i];
				auto &it = actors[i];

				vx::StringID sidMaterial = it.m_material;
				vx::StringID sidMesh = it.m_mesh;

				auto actorNameIt = scene.m_actorNames.find(sidActor);
				auto meshNameIt = scene.m_meshNames.find(sidMesh);
				auto materialNameIt = scene.m_materialNames.find(sidMaterial);

				strncpy(sceneFile->m_pActors[i].m_material, materialNameIt->c_str(), 32);
				strncpy(sceneFile->m_pActors[i].m_mesh, meshNameIt->c_str(), 32);
				strncpy(sceneFile->m_pActors[i].m_name, actorNameIt->c_str(), 32);
			}
		}

		sceneFile->m_actorCount = actorCount;
	}

	void EditorSceneToSceneFile::copyLights(const Editor::Scene &scene, SceneFile* sceneFile)
	{
		sceneFile->m_lightCount = scene.m_lightCount;
		if (scene.m_lightCount != 0)
		{

			sceneFile->m_pLights = vx::make_unique<Light[]>(scene.m_lightCount);
			for (u32 i = 0; i < scene.m_lightCount; ++i)
			{
				sceneFile->m_pLights[i] = scene.m_lights[i];
			}
		}
	}

	void EditorSceneToSceneFile::convert(const Editor::Scene &scene, SceneFile* sceneFile)
	{
		convertActors(scene, sceneFile);
		copyLights(scene, sceneFile);

		scene.m_navMesh.copy(&sceneFile->m_navMesh);

		auto meshInstanceCount = scene.m_meshInstances.size();
		sceneFile->m_meshInstanceCount = meshInstanceCount;
		if (meshInstanceCount != 0)
		{
			sceneFile->m_pMeshInstances = vx::make_unique<MeshInstanceFile[]>(meshInstanceCount);
			for (u32 i = 0; i < meshInstanceCount; ++i)
			{
				auto &it = scene.m_meshInstances[i];
				auto material = it.getMaterial();
				auto materialSid = material->getSid();

				auto instanceName = scene.getMeshInstanceName(it.getNameSid());
				auto instanceMeshName = scene.getMeshName(it.getMeshSid());
				auto instanceMaterialName = scene.getMaterialName(materialSid);
				auto instanceAnimationName = scene.getAnimationName(it.getAnimationSid());

				char meshName[32] = {};
				if (instanceMeshName)
				{
					strncpy(meshName, instanceMeshName, 32);
				}

				char materialName[32] = {};
				if (instanceMaterialName)
				{
					strncpy(materialName, instanceMaterialName, 32);
				}

				char name[32] = {};
				if (instanceName)
				{
					strncpy(name, instanceName, 32);
				}

				char animation[32] = {};
				if (instanceAnimationName)
				{
					strncpy(animation, instanceAnimationName, 32);
				}

				auto transform = it.getTransform();
				sceneFile->m_pMeshInstances[i] = MeshInstanceFile(name, meshName, materialName, animation, transform, it.getRigidBodyType());
			}
		}

		auto spawns = scene.getSpawns();
		u32 spawnCount = 0;
		if (spawns != nullptr)
		{
			spawnCount = scene.getSpawnCount();
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
		sceneFile->m_spawnCount = spawnCount;

		sceneFile->m_waypointCount = scene.m_waypointCount;
		if (scene.m_waypointCount != 0)
		{
			sceneFile->m_waypoints = vx::make_unique<Waypoint[]>(scene.m_waypointCount);
			for (u32 i = 0; i < scene.m_waypointCount; ++i)
			{
				sceneFile->m_waypoints[i] = scene.m_waypoints[i];
			}
		}

		auto jointCount = scene.m_joints.size();
		sceneFile->m_jointCount = jointCount;
		if (jointCount != 0)
		{
			sceneFile->m_joints = vx::make_unique<Joint[]>(jointCount);
			for (u32 i = 0; i < jointCount; ++i)
			{
				sceneFile->m_joints[i] = scene.m_joints[i];
			}
		}
	}
}