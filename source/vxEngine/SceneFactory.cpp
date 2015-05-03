#include "SceneFactory.h"
#include "SceneFile.h"
#include "Scene.h"
#include "FileEntry.h"
#include "MeshInstance.h"
#include <vxLib/Container/sorted_array.h>
#include <vxLib\Graphics\Mesh.h>
#include "Material.h"
#include "Light.h"
#include "enums.h"
#include "Actor.h"
#include "EditorScene.h"
#include "ConverterSceneFileToScene.h"
#include "ConverterEditorSceneToSceneFile.h"
#include "FileFactory.h"
#include "CreateSceneDescription.h"

struct SceneFactory::LoadSceneFileDescription
{
	const vx::sorted_array<vx::StringID, vx::Mesh*>* sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*>* sortedMaterials;
	std::vector<FileEntry> *pMissingFiles;
	SceneFile *pSceneFile;
};

bool SceneFactory::checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc)
{
	auto &pMeshInstances = desc.pSceneFile->getMeshInstances();
	auto meshInstanceCount = desc.pSceneFile->getNumMeshInstances();

	// check if all meshes are loaded
	U8 result = 1;
	for (U32 i = 0; i < meshInstanceCount; ++i)
	{
		auto meshFile = pMeshInstances[i].getMeshFile();
		auto meshSid = vx::make_sid(meshFile);

		// check for mesh
		auto itMesh = desc.sortedMeshes->find(meshSid);
		if (itMesh == desc.sortedMeshes->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(meshFile, FileType::Mesh));

			result = false;
		}

		// check for material
		auto materialFile = pMeshInstances[i].getMaterialFile();
		auto materialSid = vx::make_sid(materialFile);
		auto itMaterial = desc.sortedMaterials->find(materialSid);
		if (itMaterial == desc.sortedMaterials->end())
		{
			desc.pMissingFiles->push_back(FileEntry(materialFile, FileType::Material));

			result = false;
		}
	}

	auto pActors = desc.pSceneFile->getActors();
	auto actorCount = desc.pSceneFile->getActorCount();
	for (U32 i = 0; i < actorCount; ++i)
	{
		auto &actor = pActors[i];

		auto meshSid = vx::make_sid(actor.mesh);
		auto materialSid = vx::make_sid(actor.material);

		// check for mesh
		auto itMesh = desc.sortedMeshes->find(meshSid);
		if (itMesh == desc.sortedMeshes->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(actor.mesh, FileType::Mesh));

			result = false;
		}

		auto itMaterial = desc.sortedMaterials->find(materialSid);
		if (itMaterial == desc.sortedMaterials->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(actor.material, FileType::Material));

			result = false;
		}
	}

	return result;
}

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const U8* ptr, Scene *pScene)
{
	//auto result = loadSceneFile(loadDesc, ptr);

	SceneFile sceneFile;
	auto result = FileFactory::load(ptr, &sceneFile);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.sortedMaterials = desc.materials;
		loadDesc.sortedMeshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		//result = sceneFile.createScene(meshes, materials, pScene);
		result = ConverterSceneFileToScene::convert(desc.meshes, desc.materials, sceneFile, pScene);
	}

	return result;
}

bool SceneFactory::createFromFile(const Factory::CreateSceneDescription &desc, File* file, vx::StackAllocator* allocator, EditorScene *pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(file, &sceneFile, allocator);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.sortedMaterials = desc.materials;
		loadDesc.sortedMeshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		CreateEditorSceneDescription createSceneDescriptionDesc;
		createSceneDescriptionDesc.pScene = pScene;
		createSceneDescriptionDesc.sortedMaterials = desc.materials;
		createSceneDescriptionDesc.sortedMeshes = desc.meshes;
		createSceneDescriptionDesc.loadedFiles = desc.loadedFiles;
		result = sceneFile.createScene(createSceneDescriptionDesc);
	}

	return result;
}

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const U8* ptr, EditorScene* pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(ptr, &sceneFile);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.sortedMaterials = desc.materials;
		loadDesc.sortedMeshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		CreateEditorSceneDescription createSceneDescriptionDesc;
		createSceneDescriptionDesc.pScene = pScene;
		createSceneDescriptionDesc.sortedMaterials = desc.materials;
		createSceneDescriptionDesc.sortedMeshes = desc.meshes;
		createSceneDescriptionDesc.loadedFiles = desc.loadedFiles;
		result = sceneFile.createScene(createSceneDescriptionDesc);
	}

	return result;
}

bool SceneFactory::save(const EditorScene &scene, File* file)
{
	SceneFile sceneFile;
	convert(scene, &sceneFile);

	return sceneFile.saveToFile(file);
}

void SceneFactory::convert(const EditorScene &scene, SceneFile* sceneFile)
{
	ConverterEditorSceneToSceneFile::convert(scene, sceneFile);
}