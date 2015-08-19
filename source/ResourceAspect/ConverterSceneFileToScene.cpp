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
#include <vxResourceAspect/ConverterSceneFileToScene.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Scene.h>
#include <memory>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/Reference.h>
#include <vxLib/File/FileHandle.h>
#include <vxEngineLib/Joint.h>
#include <vxResourceAspect/ResourceManager.h>

namespace Converter
{
	struct SceneFileToScene::CreateSceneMeshInstancesDesc
	{
		const SceneFile *sceneFile;
		const ResourceManager<vx::MeshFile>* meshManager;
		const ResourceManager<Material>* materialManager;
		MeshInstance* pMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	struct SceneFileToScene::CreateSceneActorsDesc
	{
		const SceneFile *sceneFile;
		const ResourceManager<vx::MeshFile>* meshManager;
		const ResourceManager<Material>* materialManager;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	bool SceneFileToScene::createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc)
	{
		for (auto i = 0u; i < desc.sceneFile->m_meshInstanceCount; ++i)
		{
			auto &instance = desc.sceneFile->m_pMeshInstances[i];
			auto name = instance.getName();
			auto meshFile = instance.getMeshFile();
			auto materialFile = instance.getMaterialFile();

			auto sidName = vx::make_sid(name);
			auto sidMesh = vx::make_sid(meshFile);
			vx::StringID sidAnimation;
			auto animName = instance.getAnimation();
			if (animName[0] != '\0')
			{
				auto handle = vx::FileHandle(animName);
				sidAnimation = handle.m_sid;
			}
			auto meshPtr = desc.meshManager->find(sidMesh);
			auto sidMaterial = vx::make_sid(materialFile);
			auto materialPtr = desc.materialManager->find(sidMaterial);

			if (meshPtr == nullptr || materialPtr == nullptr)
			{
				return false;
			}

			desc.sceneMeshes->insert(sidMesh, meshPtr);
			desc.sceneMaterials->insert(sidMaterial, materialPtr);

			MeshInstanceDesc instanceDesc;
			instanceDesc.nameSid = sidName;
			instanceDesc.meshSid = sidMesh;
			instanceDesc.material = materialPtr;
			instanceDesc.animationSid = sidAnimation;
			instanceDesc.transform = instance.getTransform();
			instanceDesc.rigidBodyType = instance.getRigidBodyType();

			desc.pMeshInstances[i] = MeshInstance(instanceDesc);
		}

		return true;
	}

	bool SceneFileToScene::createSceneActors(const CreateSceneActorsDesc &desc)
	{
		auto actorCount = desc.sceneFile->m_actorCount;
		if (actorCount != 0)
		{
			desc.sceneActors->reserve(actorCount);
			for (auto i = 0u; i < actorCount; ++i)
			{
				auto &actor = desc.sceneFile->m_pActors[i];

				auto sidMesh = vx::make_sid(actor.m_mesh);
				auto meshRef = desc.meshManager->find(sidMesh);

				auto sidMaterial = vx::make_sid(actor.m_material);
				auto materialRef = desc.materialManager->find(sidMaterial);

				if (meshRef == nullptr || materialRef == nullptr)
				{
					return false;
				}

				desc.sceneMeshes->insert(sidMesh, meshRef);
				desc.sceneMaterials->insert(sidMaterial, materialRef);

				auto sidName = vx::make_sid(actor.m_name);

				Actor a;
				a.m_mesh = sidMesh;
				a.m_material = sidMaterial;
				desc.sceneActors->insert(sidName, a);
			}
		}

		return true;
	}

	bool SceneFileToScene::convert
		(
			const ResourceManager<vx::MeshFile>* meshManager,
			const ResourceManager<Material>* materialManager,
			const SceneFile &sceneFile,
			Scene* scene
			)
	{
		vx::sorted_vector<vx::StringID, Material*> sceneMaterials;
		sceneMaterials.reserve(5);

		vx::sorted_vector<vx::StringID, const vx::MeshFile*> sceneMeshes;

		auto pMeshInstances = vx::make_unique<MeshInstance[]>(sceneFile.m_meshInstanceCount);

		CreateSceneMeshInstancesDesc desc;
		desc.pMeshInstances = pMeshInstances.get();
		desc.sceneFile = &sceneFile;
		desc.sceneMaterials = &sceneMaterials;
		desc.sceneMeshes = &sceneMeshes;
		desc.meshManager = meshManager;
		desc.materialManager = materialManager;

		if (!createSceneMeshInstances(desc))
			return 0;

		vx::sorted_vector<vx::StringID, Actor> sceneActors;

		CreateSceneActorsDesc createSceneActorsDesc;
		createSceneActorsDesc.sceneActors = &sceneActors;
		createSceneActorsDesc.sceneFile = &sceneFile;
		createSceneActorsDesc.sceneMaterials = &sceneMaterials;
		createSceneActorsDesc.sceneMeshes = &sceneMeshes;
		createSceneActorsDesc.meshManager = meshManager;
		createSceneActorsDesc.materialManager = materialManager;
		if (!createSceneActors(createSceneActorsDesc))
			return 0;

		vx::sorted_vector<u32, Spawn> spawns;
		spawns.reserve(sceneFile.m_spawnCount);
		for (auto i = 0u; i < sceneFile.m_spawnCount; ++i)
		{
			Spawn spawn;
			spawn.type = sceneFile.m_pSpawns[i].type;
			spawn.position = sceneFile.m_pSpawns[i].position;
			spawn.sid = vx::make_sid(sceneFile.m_pSpawns[i].actor);

			spawns.insert(std::move(spawn.id), std::move(spawn));
		}

		u32 vertexCount = 0;
		auto indexCount = 0u;
		for (auto &it : sceneMeshes)
		{
			auto &mesh = it->getMesh();
			vertexCount += mesh.getVertexCount();
			indexCount += mesh.getIndexCount();
		}

		NavMesh navMesh;
		sceneFile.m_navMesh.copy(&navMesh);

		auto pLights = std::vector<Light>();
		pLights.reserve(sceneFile.m_lightCount);
		for (u32 i = 0; i < sceneFile.m_lightCount; ++i)
		{
			pLights.push_back(sceneFile.m_pLights[i]);
		}


		auto waypoints = std::vector<Waypoint>();
		waypoints.reserve(sceneFile.m_waypointCount);
		for (u32 i = 0; i < sceneFile.m_waypointCount; ++i)
		{
			waypoints.push_back(sceneFile.m_waypoints[i]);
		}

		auto joints = std::vector<Joint>();
		joints.reserve(sceneFile.m_jointCount);
		for (u32 i = 0; i < sceneFile.m_jointCount; ++i)
		{
			joints.push_back(sceneFile.m_joints[i]);
		}

		SceneParams sceneParams;
		sceneParams.m_baseParams.m_actors = std::move(sceneActors);
		sceneParams.m_baseParams.m_indexCount = indexCount;
		sceneParams.m_baseParams.m_lightCount = sceneFile.m_lightCount;
		sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
		sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
		sceneParams.m_baseParams.m_navMesh = std::move(navMesh);
		sceneParams.m_baseParams.m_lights = std::move(pLights);
		sceneParams.m_baseParams.m_pSpawns = std::move(spawns);
		sceneParams.m_baseParams.m_spawnCount = sceneFile.m_spawnCount;
		sceneParams.m_baseParams.m_vertexCount = vertexCount;
		sceneParams.m_meshInstanceCount = sceneFile.m_meshInstanceCount;
		sceneParams.m_pMeshInstances = std::move(pMeshInstances);
		sceneParams.m_baseParams.m_waypointCount = sceneFile.m_waypointCount;
		sceneParams.m_baseParams.m_waypoints = std::move(waypoints);
		sceneParams.m_baseParams.m_joints = std::move(joints);

		*scene = Scene(sceneParams);
		scene->sortMeshInstances();

		return true;
	}
}