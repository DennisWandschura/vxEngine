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
	const vx::sorted_array<vx::StringID64, vx::Mesh>* meshes;
	const vx::sorted_array<vx::StringID64, Material>* materials;
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
		auto itMesh = desc.meshes->find(meshSid);
		if (itMesh == desc.meshes->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(meshFile, FileType::Mesh));

			result = false;
		}

		// check for material
		auto materialFile = pMeshInstances[i].getMaterialFile();
		auto materialSid = vx::make_sid(materialFile);
		auto itMaterial = desc.materials->find(materialSid);
		if (itMaterial == desc.materials->end())
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
		auto itMesh = desc.meshes->find(meshSid);
		if (itMesh == desc.meshes->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(actor.mesh, FileType::Mesh));

			result = false;
		}

		auto itMaterial = desc.materials->find(materialSid);
		if (itMaterial == desc.materials->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(actor.material, FileType::Material));

			result = false;
		}
	}

	return result;
}

bool SceneFactory::createFromMemory(const CreateSceneDescription &desc, const U8* ptr, Scene *pScene)
{
	//auto result = loadSceneFile(loadDesc, ptr);

	SceneFile sceneFile;
	auto result = FileFactory::load(ptr, &sceneFile);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.materials = desc.materials;
		loadDesc.meshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		//result = sceneFile.createScene(meshes, materials, pScene);
		result = ConverterSceneFileToScene::convert(*desc.meshes, *desc.materials, sceneFile, pScene);
	}

	return result;
}

bool SceneFactory::createFromFile(const CreateSceneDescription &desc, File* file, vx::StackAllocator* allocator, EditorScene *pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(file, &sceneFile, allocator);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.materials = desc.materials;
		loadDesc.meshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		result = sceneFile.createScene(*desc.meshes, *desc.materials, *desc.loadedFiles, pScene);
	}

	return result;
}

bool SceneFactory::createFromMemory(const CreateSceneDescription &desc, const U8* ptr, EditorScene* pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(ptr, &sceneFile);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.materials = desc.materials;
		loadDesc.meshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		result = sceneFile.createScene(*desc.meshes, *desc.materials, *desc.loadedFiles, pScene);
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