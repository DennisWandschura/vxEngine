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
#include "ConverterSceneFile.h"
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
			auto fileActors = vx::make_unique<ActorFile[]>(actorCount);
			for (u32 i = 0; i < actorCount; ++i)
			{
				auto sidActor = keys[i];
				auto &it = actors[i];

				vx::StringID sidMaterial = it.m_material;
				vx::StringID sidMesh = it.m_mesh;

				auto actorNameIt = scene.m_actorNames.find(sidActor);
				auto meshNameIt = scene.m_meshNames.find(sidMesh);
				auto materialNameIt = scene.m_materialNames.find(sidMaterial);

				strncpy(fileActors[i].m_material, materialNameIt->c_str(), 32);
				strncpy(fileActors[i].m_mesh, meshNameIt->c_str(), 32);
				strncpy(fileActors[i].m_name, actorNameIt->c_str(), 32);
			}

			sceneFile->setActors(std::move(fileActors), actorCount);
		}
	}

	void EditorSceneToSceneFile::copyLights(const Editor::Scene &scene, SceneFile* sceneFile)
	{
		auto lightCount = scene.m_lightCount;
		if (lightCount != 0)
		{
			auto lights = vx::make_unique<Light[]>(scene.m_lightCount);
			for (u32 i = 0; i < scene.m_lightCount; ++i)
			{
				lights[i] = scene.m_lights[i];
			}

			sceneFile->setLights(std::move(lights), lightCount);
		}
	}

	void EditorSceneToSceneFile::convert(const Editor::Scene &scene, ::SceneFile* sceneFile)
	{
		SceneFile converterSceneFile(std::move(*sceneFile));

		convertActors(scene, &converterSceneFile);
		copyLights(scene, &converterSceneFile);

		converterSceneFile.setNavMesh(scene.m_navMesh);

		auto meshInstanceCount = scene.m_meshInstances.size();
		if (meshInstanceCount != 0)
		{
			auto meshInstances = vx::make_unique<MeshInstanceFile[]>(meshInstanceCount);
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
				meshInstances[i] = MeshInstanceFile(name, meshName, materialName, animation, transform, it.getRigidBodyType());
			}

			converterSceneFile.setMeshInstances(std::move(meshInstances), meshInstanceCount);
		}

		auto spawns = scene.getSpawns();
		u32 spawnCount = scene.getSpawnCount();
		if (spawnCount != 0)
		{
			auto fileSpawns = vx::make_unique<SpawnFile[]>(spawnCount);

			for (u32 i = 0; i < spawnCount; ++i)
			{
				auto &spawn = spawns[i];

				if (spawn.type != PlayerType::Human)
				{
					auto actorName = scene.getActorName(spawn.sid);
					strcpy(fileSpawns[i].actor, actorName);
				}
				else
				{
					fileSpawns[i].actor[0] = '\0';
				}

				fileSpawns[i].position = spawn.position;
				fileSpawns[i].type = spawn.type;
			}

			converterSceneFile.setSpawns(std::move(fileSpawns), spawnCount);
		}

		auto waypointCount = scene.m_waypointCount;
		if (waypointCount != 0)
		{
			auto waypoints = vx::make_unique<Waypoint[]>(scene.m_waypointCount);
			for (u32 i = 0; i < scene.m_waypointCount; ++i)
			{
				waypoints[i] = scene.m_waypoints[i];
			}

			converterSceneFile.setWaypoints(std::move(waypoints), waypointCount);
		}

		auto jointCount = scene.m_joints.size();
		if (jointCount != 0)
		{
			auto joints = vx::make_unique<Joint[]>(jointCount);
			for (u32 i = 0; i < jointCount; ++i)
			{
				joints[i] = scene.m_joints[i];
			}

			converterSceneFile.setJoints(std::move(joints), jointCount);
		}

		converterSceneFile.swap(*sceneFile);
	}
}