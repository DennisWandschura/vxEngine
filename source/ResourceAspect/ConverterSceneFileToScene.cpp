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
#include "ConverterSceneFile.h"
#include <vxResourceAspect/ConverterSceneFileToScene.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Scene.h>
#include <memory>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Graphics/Light.h>
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

	bool SceneFileToScene::createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc)
	{
		auto meshInstanceCount = desc.sceneFile->getMeshInstanceCount();
		auto meshInstances = desc.sceneFile->getMeshInstances();
		for (auto i = 0u; i < meshInstanceCount; ++i)
		{
			auto &instance = meshInstances[i];
			auto name = instance.getName();
			auto meshFile = instance.getMeshFile();
			auto materialFile = instance.getMaterialFile();

			auto sidName = vx::make_sid(name);
			auto fileHandleMesh = vx::FileHandle(meshFile);

			vx::StringID sidAnimation;
			auto animName = instance.getAnimation();
			if (animName[0] != '\0')
			{
				auto handle = vx::FileHandle(animName);
				sidAnimation = handle.m_sid;
			}
			auto meshPtr = desc.meshManager->find(fileHandleMesh.m_sid);
			auto fileHandleMaterial = vx::FileHandle(materialFile);
			auto materialPtr = desc.materialManager->find(fileHandleMaterial.m_sid);

			if (meshPtr == nullptr || materialPtr == nullptr)
			{
				return false;
			}

			desc.sceneMeshes->insert(fileHandleMesh.m_sid, meshPtr);
			desc.sceneMaterials->insert(fileHandleMaterial.m_sid, materialPtr);

			MeshInstanceDesc instanceDesc;
			instanceDesc.nameSid = sidName;
			instanceDesc.meshSid = fileHandleMesh.m_sid;
			instanceDesc.material = materialPtr;
			instanceDesc.animationSid = sidAnimation;
			instanceDesc.transform = instance.getTransform();
			instanceDesc.rigidBodyType = instance.getRigidBodyType();

			desc.pMeshInstances[i] = MeshInstance(instanceDesc);
		}

		return true;
	}

	bool SceneFileToScene::convert
		(
			const ResourceManager<vx::MeshFile>* meshManager,
			const ResourceManager<Material>* materialManager,
			::SceneFile* sceneFile,
			Scene* scene
			)
	{
		vx::sorted_vector<vx::StringID, Material*> sceneMaterials;
		sceneMaterials.reserve(5);

		vx::sorted_vector<vx::StringID, const vx::MeshFile*> sceneMeshes;

		SceneFile converterSceneFile(std::move(*sceneFile));

		auto meshInstanceCount = converterSceneFile.getMeshInstanceCount();
		auto pMeshInstances = vx::make_unique<MeshInstance[]>(meshInstanceCount);

		CreateSceneMeshInstancesDesc desc;
		desc.pMeshInstances = pMeshInstances.get();
		desc.sceneFile = &converterSceneFile;
		desc.sceneMaterials = &sceneMaterials;
		desc.sceneMeshes = &sceneMeshes;
		desc.meshManager = meshManager;
		desc.materialManager = materialManager;

		if (!createSceneMeshInstances(desc))
			return 0;

		auto spawnCount = converterSceneFile.getSpawnCount();
		auto spawns = converterSceneFile.getSpawns();
		vx::sorted_vector<u32, Spawn> sceneSpawns;
		sceneSpawns.reserve(spawnCount);
		for (auto i = 0u; i < spawnCount; ++i)
		{
			Spawn spawn;
			spawn.type = spawns[i].type;
			spawn.position = spawns[i].position;
			spawn.actorSid = vx::FileHandle(spawns[i].actor).m_sid;

			sceneSpawns.insert(std::move(spawn.id), std::move(spawn));
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
		converterSceneFile.getNavMesh().copy(&navMesh);

		auto lightCount = converterSceneFile.getLightCount();
		auto lights = converterSceneFile.getLights();
		auto pLights = std::vector<Graphics::Light>();
		pLights.reserve(lightCount);
		for (u32 i = 0; i < lightCount; ++i)
		{
			pLights.push_back(lights[i]);
		}

		auto waypointCount = converterSceneFile.getWaypointCount();
		auto waypoints = converterSceneFile.getWaypoints();
		auto sceneWaypoints = std::vector<Waypoint>();
		sceneWaypoints.reserve(waypointCount);
		for (u32 i = 0; i < waypointCount; ++i)
		{
			sceneWaypoints.push_back(waypoints[i]);
		}

		auto jointCount = converterSceneFile.getJointCount();
		auto joints = converterSceneFile.getJoints();
		auto sceneJoints = std::vector<Joint>();
		sceneJoints.reserve(jointCount);
		for (u32 i = 0; i < jointCount; ++i)
		{
			sceneJoints.push_back(joints[i]);
		}

		SceneParams sceneParams;
		sceneParams.m_baseParams.m_indexCount = indexCount;
		sceneParams.m_baseParams.m_lightCount = lightCount;
		sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
		sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
		sceneParams.m_baseParams.m_navMesh = std::move(navMesh);
		sceneParams.m_baseParams.m_lights = std::move(pLights);
		sceneParams.m_baseParams.m_pSpawns = std::move(sceneSpawns);
		sceneParams.m_baseParams.m_spawnCount = spawnCount;
		sceneParams.m_baseParams.m_vertexCount = vertexCount;
		sceneParams.m_meshInstanceCount = meshInstanceCount;
		sceneParams.m_pMeshInstances = std::move(pMeshInstances);
		sceneParams.m_baseParams.m_waypointCount = waypointCount;
		sceneParams.m_baseParams.m_waypoints = std::move(sceneWaypoints);
		sceneParams.m_baseParams.m_joints = std::move(sceneJoints);

		*scene = Scene(sceneParams);
		scene->sortMeshInstances();

		return true;
	}
}