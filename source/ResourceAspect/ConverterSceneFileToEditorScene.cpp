#include <vxResourceAspect/ConverterSceneFileToEditorScene.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/SceneFile.h>
#include <vxLib/Container/sorted_array.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxLib/File/FileHandle.h>
#include <vxEngineLib/Reference.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Waypoint.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/MeshFile.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/Joint.h>
#include "ConverterSceneFile.h"

#include <vxResourceAspect/ResourceManager.h>
#include <vxResourceAspect/ResourceAspect.h>

namespace Converter
{
	struct CreateSceneShared
	{
		const ResourceManager<vx::MeshFile>* meshData;
		const ResourceManager<Material>* materialData;
		const ResourceManager<vx::Animation>* animationData;
		MeshInstance* pMeshInstances;
		const MeshInstanceFileV8* meshInstancesFile;
		vx::sorted_vector<vx::StringID, Editor::MeshInstance>* sortedMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
		vx::sorted_vector<vx::StringID, vx::Animation*>* sceneAnimations;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		vx::sorted_vector<vx::StringID, std::string>* sceneMeshInstanceNames;
		Spawn* sceneSpawns;
		const SpawnFile* spawnsSrc;
		const ActorFile* actorsSrc;
		u32* vertexCount;
		u32* indexCount;
		u32 instanceCount;
		u32 spawnCount;
		u32 actorCount;
	};

	struct CreateSceneMeshInstancesDesc
	{
		const ResourceManager<vx::MeshFile>* meshData;
		const ResourceManager<Material>* materialData;
		const ResourceManager<vx::Animation>* animationData;
		MeshInstance* pMeshInstances;
		const MeshInstanceFileV8* meshInstancesFile;
		vx::sorted_vector<vx::StringID, Editor::MeshInstance>* sortedMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
		vx::sorted_vector<vx::StringID, vx::Animation*>* sceneAnimations;
		vx::sorted_vector<vx::StringID, std::string>* sceneMeshInstanceNames;
		u32 instanceCount;
	};

	struct CreateSceneActorsDesc
	{
		const ResourceManager<vx::MeshFile>* meshData;
		const ResourceManager<Material>* materialData;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		vx::sorted_vector<vx::StringID, const vx::MeshFile*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
		const ActorFile* actorsSrc;
		u32 actorCount;
	};

	struct InstanceInserter
	{
		typedef void(InstanceInserter::*InserterFun)(const MeshInstance &instance, u32 i, const char* name);

		InserterFun m_fp;
		MeshInstance* m_instances;
		vx::sorted_vector<vx::StringID, Editor::MeshInstance>* m_sortedInstances;

		explicit InstanceInserter(const CreateSceneMeshInstancesDesc &desc)
			:m_instances(desc.pMeshInstances),
			m_sortedInstances(desc.sortedMeshInstances)
		{
			if (m_instances == nullptr)
			{
				m_fp = &InstanceInserter::insertSorted;
			}
			else
			{
				m_fp = &InstanceInserter::insertArray;
			}
		}

		void insertArray(const MeshInstance &instance, u32 i, const char*)
		{
			m_instances[i] = instance;
		}

		void insertSorted(const MeshInstance &instance, u32, const char* name)
		{
			Editor::MeshInstance editorInstance(instance, std::string(name));
			m_sortedInstances->insert(instance.getNameSid(), editorInstance);
		}

		void operator()(const MeshInstance &instance, u32 i, const char* name)
		{
			(this->*m_fp)(instance, i, name);
		}
	};

	bool createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc)
	{
		InstanceInserter instanceInserter(desc);

		for (auto i = 0u; i < desc.instanceCount; ++i)
		{
			std::string genName = "instance" + std::to_string(i);

			auto &instanceFile = desc.meshInstancesFile[i];
			auto name = instanceFile.getName();
			auto meshFile = instanceFile.getMeshFile();
			auto materialFile = instanceFile.getMaterialFile();
			auto animationFile = instanceFile.getAnimation();

			if (name[0] == '\0')
				name = genName.c_str();

			vx::FileHandle sidAnimation;
			if (animationFile[0] != '\0')
				sidAnimation = vx::FileHandle(animationFile);

			auto sidName = vx::FileHandle(name);

			auto sidMesh = vx::FileHandle(meshFile);
			auto itMesh = desc.meshData->find(sidMesh.m_sid);

			auto sidMaterial = vx::FileHandle(materialFile);
			auto itMaterial = desc.materialData->find(sidMaterial.m_sid);

			auto itAnimation = desc.animationData->find(sidAnimation.m_sid);

			if (itMesh == nullptr || itMaterial == nullptr)
			{
				return false;
			}

			if (sidAnimation.m_sid.value != 0 && itAnimation == nullptr)
			{
				return false;
			}

			desc.sceneMeshes->insert(sidMesh.m_sid, itMesh);
			desc.sceneMaterials->insert(sidMaterial.m_sid, itMaterial);
			if (sidAnimation.m_sid.value != 0)
			{
				desc.sceneAnimations->insert(sidAnimation.m_sid, itAnimation);
			}

			MeshInstanceDesc instanceDesc;
			instanceDesc.nameSid = sidName.m_sid;
			instanceDesc.meshSid = sidMesh.m_sid;
			instanceDesc.material = itMaterial;
			instanceDesc.animationSid = sidAnimation.m_sid;
			instanceDesc.transform = instanceFile.getTransform();
			instanceDesc.rigidBodyType = instanceFile.getRigidBodyType();

			auto instance = MeshInstance(instanceDesc);
			instanceInserter(instance, i, name);

			if (desc.sceneMeshInstanceNames)
			{
				desc.sceneMeshInstanceNames->insert(sidName.m_sid, std::string(name));
			}
		}

		return true;
	}

	bool createSceneActorsNew(const CreateSceneActorsDesc &desc)
	{
		if (desc.actorCount != 0)
		{
			desc.sceneActors->reserve(desc.actorCount);
			for (auto i = 0u; i < desc.actorCount; ++i)
			{
				auto &actor = desc.actorsSrc[i];

				auto sidMesh = vx::FileHandle(actor.m_mesh);
				auto itMesh = desc.meshData->find(sidMesh.m_sid);

				auto sidMaterial = vx::FileHandle(actor.m_material);
				auto itMaterial = desc.materialData->find(sidMaterial.m_sid);

				if (itMesh == nullptr || itMaterial == nullptr)
				{
					return false;
				}

				desc.sceneMeshes->insert(sidMesh.m_sid, itMesh);
				desc.sceneMaterials->insert(sidMaterial.m_sid, itMaterial);

				auto sidName = vx::FileHandle(actor.m_name).m_sid;

				Actor a;
				a.m_mesh = sidMesh.m_sid;
				a.m_material = sidMaterial.m_sid;
				desc.sceneActors->insert(sidName, a);
			}
		}

		return true;
	}

	bool createSceneSharedNew(const CreateSceneShared &desc)
	{
		CreateSceneMeshInstancesDesc createMeshInstancesDesc;
		createMeshInstancesDesc.pMeshInstances = desc.pMeshInstances;
		createMeshInstancesDesc.sortedMeshInstances = desc.sortedMeshInstances;
		createMeshInstancesDesc.sceneMaterials = desc.sceneMaterials;
		createMeshInstancesDesc.sceneAnimations = desc.sceneAnimations;
		createMeshInstancesDesc.sceneMeshes = desc.sceneMeshes;
		createMeshInstancesDesc.materialData = desc.materialData;
		createMeshInstancesDesc.meshData = desc.meshData;
		createMeshInstancesDesc.animationData = desc.animationData;
		createMeshInstancesDesc.sceneMeshInstanceNames = desc.sceneMeshInstanceNames;
		createMeshInstancesDesc.instanceCount = desc.instanceCount;
		createMeshInstancesDesc.meshInstancesFile = desc.meshInstancesFile;
		if (!createSceneMeshInstances(createMeshInstancesDesc))
		{
			printf("SceneFile::createSceneShared::createSceneMeshInstances error\n");
			return false;
		}

		CreateSceneActorsDesc createActorDesc;
		createActorDesc.sceneActors = desc.sceneActors;
		createActorDesc.sceneMaterials = desc.sceneMaterials;
		createActorDesc.sceneMeshes = desc.sceneMeshes;
		createActorDesc.materialData = desc.materialData;
		createActorDesc.meshData = desc.meshData;
		createActorDesc.actorCount = desc.actorCount;
		createActorDesc.actorsSrc = desc.actorsSrc;
		if (!createSceneActorsNew(createActorDesc))
		{
			printf("SceneFile::createSceneShared::createSceneActors error\n");
			return false;
		}

		for (auto i = 0u; i < desc.spawnCount; ++i)
		{
			desc.sceneSpawns[i].type = desc.spawnsSrc[i].type;
			desc.sceneSpawns[i].position = desc.spawnsSrc[i].position;
			desc.sceneSpawns[i].sid = vx::FileHandle(desc.spawnsSrc[i].actor).m_sid;
		}

		*desc.vertexCount = 0;
		*desc.indexCount = 0u;
		for (auto &it : *desc.sceneMeshes)
		{
			auto &mesh = it->getMesh();
			*desc.vertexCount += mesh.getVertexCount();
			*desc.indexCount += mesh.getIndexCount();
		}

		return true;
	}

	bool SceneFileToEditorScene::convert(::SceneFile* src, const CreateEditorSceneDescription &desc)
	{
		SceneFile converterSceneFile(std::move(*src));

		vx::sorted_vector<vx::StringID, Material*> sceneMaterials;
		sceneMaterials.reserve(5);

		vx::sorted_vector<vx::StringID, vx::Animation*> sceneAnimations;

		vx::sorted_vector<vx::StringID, const vx::MeshFile*> sceneMeshes;
		vx::sorted_vector<vx::StringID, Actor> sceneActors;

		auto instanceCount = converterSceneFile.getMeshInstanceCount();
		vx::sorted_vector<vx::StringID, std::string> sceneMeshInstanceNames;
		vx::sorted_vector<vx::StringID, Editor::MeshInstance> meshInstances;
		meshInstances.reserve(instanceCount);

		auto spawnCount = converterSceneFile.getSpawnCount();
		auto sceneSpawns = vx::make_unique<Spawn[]>(spawnCount);

		auto actorCount = converterSceneFile.getActorCount();
		auto actorsSrc = converterSceneFile.getActors();

		u32 vertexCount = 0;
		u32 indexCount = 0;

		CreateSceneShared sharedDesc;
		sharedDesc.pMeshInstances = nullptr;
		sharedDesc.sortedMeshInstances = &meshInstances;
		sharedDesc.sceneMaterials = &sceneMaterials;
		sharedDesc.sceneAnimations = &sceneAnimations;
		sharedDesc.sceneMeshes = &sceneMeshes;
		sharedDesc.sceneActors = &sceneActors;
		sharedDesc.sceneSpawns = sceneSpawns.get();
		sharedDesc.materialData = desc.materialData;
		sharedDesc.meshData = desc.meshData;
		sharedDesc.animationData = desc.animationData;
		sharedDesc.vertexCount = &vertexCount;
		sharedDesc.indexCount = &indexCount;
		sharedDesc.sceneMeshInstanceNames = &sceneMeshInstanceNames;
		sharedDesc.instanceCount = instanceCount;
		sharedDesc.meshInstancesFile = converterSceneFile.getMeshInstances();
		sharedDesc.spawnsSrc = converterSceneFile.getSpawns();
		sharedDesc.spawnCount = spawnCount;
		sharedDesc.actorCount = actorCount;
		sharedDesc.actorsSrc = actorsSrc;
		if (!createSceneSharedNew(sharedDesc))
		{
			printf("SceneFile::createScene error\n");
			return false;
		}

		auto materialCount = sceneMaterials.size();
		vx::sorted_vector<vx::StringID, std::string> materialNames;
		materialNames.reserve(materialCount);
		for (u32 i = 0; i < materialCount; ++i)
		{
			auto sid = sceneMaterials.keys()[i];

			auto it = desc.materialData->getName(sid);

			//char str[32];
			//strncpy(str, it->c_str(), 32);

			materialNames.insert(sid, it);
		}

		auto meshCount = sceneMeshes.size();
		vx::sorted_vector<vx::StringID, std::string> meshNames;
		meshNames.reserve(meshCount);
		for (u32 i = 0; i < meshCount; ++i)
		{
			auto sid = sceneMeshes.keys()[i];

			auto it = desc.meshData->getName(sid);

			//char str[32];
			//strncpy(str, it->c_str(), 32);

			meshNames.insert(sid, it);
		}

		vx::sorted_vector<vx::StringID, std::string> actorNames;
		actorNames.reserve(actorCount);
		for (u32 i = 0; i < actorCount; ++i)
		{
			auto &actorFile = actorsSrc[i];

			auto sid = vx::FileHandle(actorFile.m_name).m_sid;
			actorNames.insert(sid, actorFile.m_name);
		}

		auto lightCount = converterSceneFile.getLightCount();
		auto lights = converterSceneFile.getLights();

		auto &fileNavMesh = converterSceneFile.getNavMesh();

		Editor::SceneParams sceneParams;
		sceneParams.m_baseParams.m_actors = std::move(sceneActors);
		sceneParams.m_baseParams.m_indexCount = indexCount;
		sceneParams.m_baseParams.m_lightCount = lightCount;
		sceneParams.m_baseParams.m_materials = std::move(sceneMaterials);
		sceneParams.m_baseParams.m_animations = std::move(sceneAnimations);
		sceneParams.m_baseParams.m_meshes = std::move(sceneMeshes);
		sceneParams.m_baseParams.m_navMesh.swap(fileNavMesh);
		sceneParams.m_meshInstanceNames = std::move(sceneMeshInstanceNames);

		sceneParams.m_baseParams.m_lights.reserve(lightCount);
		for (u32 i = 0; i < lightCount; ++i)
		{
			sceneParams.m_baseParams.m_lights.push_back(lights[i]);
		}

		sceneParams.m_baseParams.m_pSpawns.reserve(spawnCount);
		for (u32 i = 0; i < lightCount; ++i)
		{
			sceneParams.m_baseParams.m_pSpawns.insert(std::move(sceneSpawns[i].id), std::move(sceneSpawns[i]));
		}

		sceneParams.m_baseParams.m_spawnCount = spawnCount;
		sceneParams.m_baseParams.m_vertexCount = vertexCount;
		sceneParams.m_meshInstances = std::move(meshInstances);

		auto waypointCount = converterSceneFile.getWaypointCount();
		auto waypoints = converterSceneFile.getWaypoints();
		sceneParams.m_baseParams.m_waypointCount = waypointCount;
		std::vector<Waypoint> waypointsEditor;
		waypointsEditor.reserve(waypointCount);
		for (u32 i = 0; i < waypointCount; ++i)
		{
			waypointsEditor.push_back(waypoints[i]);
		}
		sceneParams.m_baseParams.m_waypoints = std::move(waypointsEditor);

		sceneParams.m_materialNames = std::move(materialNames);
		sceneParams.m_meshNames = std::move(meshNames);
		sceneParams.m_actorNames = std::move(actorNames);

		auto jointCount = converterSceneFile.getJointCount();
		auto srcJoints = converterSceneFile.getJoints();
		auto joints = std::vector<Joint>();
		joints.reserve(jointCount);
		for (u32 i = 0; i < jointCount; ++i)
		{
			joints.push_back(srcJoints[i]);
		}
		sceneParams.m_baseParams.m_joints = std::move(joints);

		*desc.pScene = Editor::Scene(std::move(sceneParams));
		desc.pScene->sortMeshInstances();

		converterSceneFile.swap(*src);

		return true;
	}
}