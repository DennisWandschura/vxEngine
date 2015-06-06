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
	const vx::sorted_array<vx::StringID, vx::MeshFile*>* sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*>* sortedMaterials;
	std::vector<FileEntry> *pMissingFiles;
	SceneFile *pSceneFile;
};

bool SceneFactory::checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc)
{
	auto &pMeshInstances = desc.pSceneFile->getMeshInstances();
	auto meshInstanceCount = desc.pSceneFile->getNumMeshInstances();

	// check if all meshes are loaded
	u8 result = 1;
	for (u32 i = 0; i < meshInstanceCount; ++i)
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
			desc.pMissingFiles->push_back(FileEntry(actor.m_mesh, FileType::Mesh));

			result = false;
		}

		auto itMaterial = desc.sortedMaterials->find(materialSid);
		if (itMaterial == desc.sortedMaterials->end())
		{
			// request load
			desc.pMissingFiles->push_back(FileEntry(actor.m_material, FileType::Material));

			result = false;
		}
	}

	return result;
}

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, Scene *pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(ptr, &sceneFile, nullptr);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.sortedMaterials = desc.materials;
		loadDesc.sortedMeshes = desc.meshes;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}
	else
	{
		printf("Failed loading scene file (wrong header/crc)\n");
	}

	if (result)
	{
		result = ConverterSceneFileToScene::convert(desc.meshes, desc.materials, sceneFile, pScene);
	}
	else
	{
		printf("Scene missing dependencies\n");
	}

	if (!result)
	{
		printf("Error converting SceneFile to Scene\n");
	}

	return result;
}

bool SceneFactory::createFromFile(const Factory::CreateSceneDescription &desc, vx::File* file, vx::StackAllocator* allocator, Editor::Scene *pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(file, &sceneFile, allocator, nullptr);

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

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, Editor::Scene* pScene)
{
	SceneFile sceneFile;
	auto result = FileFactory::load(ptr, &sceneFile, nullptr);

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

bool SceneFactory::save(const Editor::Scene &scene, vx::File* file)
{
	SceneFile sceneFile;
	convert(scene, &sceneFile);

	return sceneFile.saveToFile(file);
}

void SceneFactory::convert(const Editor::Scene &scene, SceneFile* sceneFile)
{
	ConverterEditorSceneToSceneFile::convert(scene, sceneFile);
}

void SceneFactory::deleteScene(Editor::Scene *scene)
{
	delete(scene);
}