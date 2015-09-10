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
#include <vxResourceAspect/ConverterSceneFileToEditorScene.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/EditorMeshInstance.h>
#include <vxResourceAspect/ConverterEditorSceneToSceneFile.h>
#include <vxEngineLib/FileFactory.h>

namespace SceneFactoryCpp
{
	class SceneFileLoad : public SceneFile
	{
	public:
		SceneFileLoad(SceneFile &&rhs) :SceneFile(std::move(rhs)) {}
		~SceneFileLoad() {}

		const MeshInstanceFileV8* getMeshInstances() const { return m_pMeshInstances.get(); }
		u32 getMeshInstanceCount() const { return m_meshInstanceCount; }

		u32 getActorCount() const { return m_actorCount; }
		const ActorFile* getActors() const { return m_pActors.get(); }

		void swap(SceneFile &rhs)
		{
			SceneFile::swap(rhs);
		}
	};

	bool checkMeshInstanceMesh(const vx::FileEntry &meshFileEntry, const vx::sorted_array<vx::StringID, vx::MeshFile*>* sortedMeshes, std::vector<vx::FileEntry>* missingFiles)
	{
		bool result = true;
		auto itMesh = sortedMeshes->find(meshFileEntry.getSid());
		if (itMesh == sortedMeshes->end())
		{
			// request load
			missingFiles->push_back(meshFileEntry);

			//printf("could not find mesh: %s %llu\n", meshFile, meshFileEntry.getSid());

			result = false;
		}

		return result;
	}

	bool checkMeshInstanceMaterial(const vx::FileEntry &materialFileEntry, const vx::sorted_array<vx::StringID, Material*>* sortedMaterials, std::vector<vx::FileEntry>* missingFiles)
	{
		bool result = true;
		auto it = sortedMaterials->find(materialFileEntry.getSid());
		if (it == sortedMaterials->end())
		{
			// request load
			missingFiles->push_back(materialFileEntry);

			//printf("could not find mesh: %s %llu\n", meshFile, meshFileEntry.getSid());

			result = false;
		}

		return result;
	}

	bool checkMeshInstanceAnimation(const vx::FileEntry &animationFileEntry, const vx::sorted_array<vx::StringID, vx::Animation*>* sortedAnimations, std::vector<vx::FileEntry>* missingFiles)
	{
		bool result = true;

		auto it = sortedAnimations->find(animationFileEntry.getSid());
		if (it == sortedAnimations->end())
		{
			// request load
			missingFiles->push_back(animationFileEntry);

			//printf("could not find mesh: %s %llu\n", meshFile, meshFileEntry.getSid());

			result = false;
		}

		return result;
	}

	template<typename T>
	void checkResource(const ResourceManager<T>* resManager, const vx::FileEntry &fileEntry, vx::sorted_vector<vx::StringID, vx::FileEntry>* missingFiles, bool* result)
	{
		auto sid = fileEntry.getSid();
		auto meshRef = resManager->find(sid);
		if (meshRef == nullptr)
		{
			auto itFile = missingFiles->find(sid);
			if (itFile == missingFiles->end())
			{
				missingFiles->insert(sid, fileEntry);
			}
			*result = false;
		}
	}

	bool checkMeshInstances
		(
			const ResourceManager<vx::MeshFile>* meshManager,
			const ResourceManager<Material>* materialManager,
			const ResourceManager<vx::Animation>* animationManager,
			const MeshInstanceFileV8* instances,
			u32 count,
			vx::sorted_vector<vx::StringID, vx::FileEntry>* missingFiles
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
		const MeshInstanceFileV8* meshInstances;
		u32 instanceCount;
		vx::sorted_vector<vx::StringID, vx::FileEntry>* missingFiles;
		const ActorFile* actorFiles;
		u32 actorCount;
	};

	bool checkActors(const CheckAssetsDesc &desc)
	{
		bool result = true;

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

	bool checkIfAssetsAreLoaded(const CheckAssetsDesc &desc)
	{
		//printf("checking mesh instances\n");
		bool result = true;

		if (!checkMeshInstances(desc.meshManager, desc.materialManager, desc.animationManager, desc.meshInstances, desc.instanceCount, desc.missingFiles))
		{
			result = false;
			//printf("missing instance assets\n");
		}

		if (!checkActors(desc))
		{
			result = false;
		}

		return result;
	}

	bool checkIfAssetsAreLoaded(const Factory::CreateSceneDesc &desc, SceneFile &&sceneFile)
	{
		SceneFactoryCpp::SceneFileLoad loadedSceneFile(std::move(sceneFile));

		SceneFactoryCpp::CheckAssetsDesc checkDesc
		{
			desc.meshManager,
			desc.materialManager,
			desc.animationManager,
			loadedSceneFile.getMeshInstances(),
			loadedSceneFile.getMeshInstanceCount(),
			desc.missingFiles,
			loadedSceneFile.getActors(),
			loadedSceneFile.getActorCount()
		};

		auto result = SceneFactoryCpp::checkIfAssetsAreLoaded(checkDesc);

		loadedSceneFile.swap(sceneFile);

		return result;
	}
}

struct SceneFactory::LoadSceneFileDescription
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*>* sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*>* sortedMaterials;
	const vx::sorted_array<vx::StringID, vx::Animation*>* sortedAnimations;
	std::vector<vx::FileEntry> *pMissingFiles;
	SceneFactoryCpp::SceneFileLoad *pSceneFile;
};

bool SceneFactory::checkMeshInstances(const LoadSceneFileDescription &desc, const MeshInstanceFileV8* instances, u32 count)
{
	bool result = true;
	for (u32 i = 0; i < count; ++i)
	{
		auto &instance = instances[i];
		auto meshFile = instances[i].getMeshFile();
		auto meshFileEntry = vx::FileEntry(meshFile, vx::FileType::Mesh);

		// check for mesh
		if (!SceneFactoryCpp::checkMeshInstanceMesh(meshFileEntry, desc.sortedMeshes, desc.pMissingFiles))
		{
			result = false;
		}

		// check for material
		auto materialFile = instances[i].getMaterialFile();
		auto materialFileEntry = vx::FileEntry(materialFile, vx::FileType::Material);
		if (!SceneFactoryCpp::checkMeshInstanceMaterial(materialFileEntry, desc.sortedMaterials, desc.pMissingFiles))
		{
			result = false;
		}

		auto animationName = instances[i].getAnimation();
		if (animationName[0] != '\0')
		{
			auto animationFileEntry = vx::FileEntry(animationName, vx::FileType::Animation);
			if (!SceneFactoryCpp::checkMeshInstanceAnimation(animationFileEntry, desc.sortedAnimations, desc.pMissingFiles))
			{
				result = false;
			}
		}
	}

	return result;
}

bool SceneFactory::checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc)
{
	auto pMeshInstances = desc.pSceneFile->getMeshInstances();
	auto instanceCount = desc.pSceneFile->getMeshInstanceCount();

	//printf("checking mesh instances\n");
	bool result = checkMeshInstances(desc, pMeshInstances, instanceCount);
	if (!result)
	{
		//printf("missing instance assets\n");
	}

	auto pActors = desc.pSceneFile->getActors();
	auto actorCount = desc.pSceneFile->getActorCount();

	//printf("checking actors: %u\n", actorCount);
	for (u32 i = 0; i < actorCount; ++i)
	{
		auto &actor = pActors[i];

		auto meshSid = vx::make_sid(actor.m_mesh);
		auto materialSid = vx::make_sid(actor.m_material);

		// check for mesh
		auto itMesh = desc.sortedMeshes->find(meshSid);
		if (itMesh == desc.sortedMeshes->end())
		{
			// request load
			desc.pMissingFiles->push_back(vx::FileEntry(actor.m_mesh, vx::FileType::Mesh));

			result = false;
		}

		auto itMaterial = desc.sortedMaterials->find(materialSid);
		if (itMaterial == desc.sortedMaterials->end())
		{
			// request load
			desc.pMissingFiles->push_back(vx::FileEntry(actor.m_material, vx::FileType::Material));

			result = false;
		}
	}

	return result;
}

bool SceneFactory::createFromMemory(const Factory::CreateSceneDesc &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Scene *pScene)
{
	auto marker = scratchAllocator->getMarker();
	bool result = false;
	SceneFile sceneFile = FileFactory::load(ptr, fileSize, &result, scratchAllocator);

	if (result)
	{
		result = SceneFactoryCpp::checkIfAssetsAreLoaded(desc, std::move(sceneFile));
	}
	else
	{
		printf("SceneFactory::createFromMemory failed\n");
	}

	if (result)
	{
		result = Converter::SceneFileToScene::convert(desc.meshManager, desc.materialManager, &sceneFile, pScene);
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

bool SceneFactory::createFromMemory(const Factory::CreateSceneDesc &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Editor::Scene *pScene)
{
	auto marker = scratchAllocator->getMarker();
	bool result = false;
	SceneFile sceneFile = FileFactory::load(ptr, fileSize, &result, scratchAllocator);

	if (result)
	{
		result = SceneFactoryCpp::checkIfAssetsAreLoaded(desc, std::move(sceneFile));
	}

	if (result)
	{
		CreateEditorSceneDescription loadDesc;
		loadDesc.animationData = desc.animationManager;
		loadDesc.materialData = desc.materialManager;
		loadDesc.meshData = desc.meshManager;
		loadDesc.pScene = pScene;
		result = Converter::SceneFileToEditorScene::convert(&sceneFile, loadDesc);
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

void SceneFactory::saveToFile(const Editor::Scene &scene, vx::File* f)
{
	SceneFile sceneFile(SceneFile::getGlobalVersion());
	convert(scene, &sceneFile);

	auto sceneInstanceCount = scene.getMeshInstanceCount();
	auto sceneInstances = scene.getMeshInstancesEditor();
	for (u32 i = 0; i < sceneInstanceCount; ++i)
	{
		auto sid = sceneInstances[i].getAnimationSid();
		if (sid.value != 0)
		{
			printf("SceneInstance Animation: %llu\n", sid.value);
		}
	}

	SceneFactoryCpp::SceneFileLoad loadedSceneFile(std::move(sceneFile));

	auto sceneFileInstanceCount = loadedSceneFile.getMeshInstanceCount();
	auto sceneFileInstances = loadedSceneFile.getMeshInstances();
	for (u32 i = 0; i < sceneFileInstanceCount; ++i)
	{
		auto str = sceneFileInstances[i].getAnimation();
		if (str[0] != '\0')
		{
			printf("SceneFile Animation: %s\n", str);
		}
	}

	loadedSceneFile.swap(sceneFile);

	vx::FileFactory::saveToFile(f, &sceneFile);
}

void SceneFactory::convert(const Editor::Scene &scene, SceneFile* sceneFile)
{
	Converter::EditorSceneToSceneFile::convert(scene, sceneFile);
}

void SceneFactory::deleteScene(Editor::Scene *scene)
{
	delete(scene);
}