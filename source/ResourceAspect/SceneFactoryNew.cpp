#include <vxResourceAspect/SceneFactory.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxEngineLib/SceneFile.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxResourceAspect/FileEntry.h>
#include <vxEngineLib/Actor.h>
#include <vxResourceAspect/FileFactory.h>
#include <vxResourceAspect/ConverterSceneFileToScene.h>
#include <vxEngineLib/Animation.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/MeshFile.h>

namespace SceneFactoryCpp
{
	template<typename T>
	void checkResource(const ResourceManager<T>* resManager, const vx::FileEntry &fileEntry, std::vector<vx::FileEntry>* missingFiles, bool* result)
	{
		auto meshRef = resManager->find(fileEntry.getSid());
		if (meshRef == nullptr)
		{
			missingFiles->push_back(fileEntry);
			*result = false;
		}
	}

	bool checkMeshInstances
		(
			const ResourceManager<vx::MeshFile>* meshManager,
			const ResourceManager<Material>* materialManager,
			const ResourceManager<vx::Animation>* animationManager,
			const MeshInstanceFile* instances,
			u32 count,
			std::vector<vx::FileEntry>* missingFiles
			)
	{

		bool result = true;
		for (u32 i = 0; i < count; ++i)
		{
			auto &instance = instances[i];
			auto meshFile = instances[i].getMeshFile();
			auto meshFileEntry = vx::FileEntry(meshFile, vx::FileType::Mesh);

			// check for mesh
			checkResource(meshManager, meshFileEntry, missingFiles, &result);

			// check for material
			auto materialFile = instances[i].getMaterialFile();
			auto materialFileEntry = vx::FileEntry(materialFile, vx::FileType::Material);

			checkResource(materialManager, materialFileEntry, missingFiles, &result);

			auto animationName = instances[i].getAnimation();
			if (animationName[0] != '\0')
			{
				auto animationFileEntry = vx::FileEntry(animationName, vx::FileType::Animation);

				checkResource(animationManager, animationFileEntry, missingFiles, &result);
			}
		}

		return result;
	}

	struct CheckAssetsDesc
	{
		const ResourceManager<vx::MeshFile>* meshManager;
		const ResourceManager<Material>* materialManager;
		const ResourceManager<vx::Animation>* animationManager;
		const MeshInstanceFile* meshInstances;
		u32 instanceCount;
		std::vector<vx::FileEntry>* missingFiles;
		const ActorFile* actorFiles;
		u32 actorCount;
	};

	bool checkIfAssetsAreLoaded
		(
			const CheckAssetsDesc &desc
			)
	{
		//printf("checking mesh instances\n");
		bool result = checkMeshInstances(desc.meshManager, desc.materialManager, desc.animationManager, desc.meshInstances, desc.instanceCount, desc.missingFiles);
		if (!result)
		{
			//printf("missing instance assets\n");
		}

		//auto pActors = desc.pSceneFile->getActors();
		//auto actorCount = desc.pSceneFile->getActorCount();

		//printf("checking actors: %u\n", actorCount);
		for (u32 i = 0; i < desc.actorCount; ++i)
		{
			auto &actorFile = desc.actorFiles[i];

			auto entryMesh = vx::FileEntry(actorFile.m_mesh, vx::FileType::Mesh);
			auto entryMaterial = vx::FileEntry(actorFile.m_material, vx::FileType::Material);
			//	auto meshSid = vx::make_sid(actorFile.m_mesh);
				//auto materialSid = vx::make_sid(actorFile.m_material);

			// check for mesh
			checkResource(desc.meshManager, entryMesh, desc.missingFiles, &result);
			checkResource(desc.materialManager, entryMaterial, desc.missingFiles, &result);
		}

		return result;
	}
}

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescNew &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Scene *pScene)
{
	auto marker = scratchAllocator->getMarker();
	bool result = false;
	SceneFile sceneFile = FileFactory::load(ptr, fileSize, &result, scratchAllocator);

	if (result)
	{
		//vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Loaded Scene File version %u", sceneFile.getVersion());

		
		SceneFactoryCpp::CheckAssetsDesc checkDesc
		{
			desc.meshManager,
			desc.materialManager,
			desc.animationManager,
			sceneFile.getMeshInstances(),
			sceneFile.getNumMeshInstances(),
			desc.missingFiles,
			sceneFile.getActors(),
			sceneFile.getActorCount()
		};

		result = SceneFactoryCpp::checkIfAssetsAreLoaded(checkDesc);
	}
	else
	{
		//printf("Failed loading scene file (wrong header/crc)\n");
	}

	if (result)
	{
		result = ConverterSceneFileToScene::convert(desc.meshManager, desc.materialManager, sceneFile, pScene);
	}
	else
	{
		//printf("Scene missing dependencies\n");
	}

	if (!result)
	{
		//printf("Error converting SceneFile to Scene\n");
	}

	scratchAllocator->clear(marker);

	return result;
}