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
#include <vxResourceAspect/SceneFactory.h>
#include <vxEngineLib/SceneFile.h>
#include <vxResourceAspect/ConverterSceneFileToScene.h>
#include <vxResourceAspect/ConverterEditorSceneToSceneFile.h>
#include <vxResourceAspect/FileFactory.h>
#include <vxResourceAspect/FileEntry.h>
#include <vxResourceAspect/CreateSceneDescription.h>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/Graphics/Mesh.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/Light.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/EditorScene.h>
#include <vxEngineLib/FileFactory.h>
#include <vxEngineLib/MeshInstanceFile.h>
#include <vxEngineLib/debugPrint.h>
#include <vxEngineLib/Reference.h>

struct SceneFactory::LoadSceneFileDescription
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*>* sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>>* sortedMaterials;
	std::vector<vx::FileEntry> *pMissingFiles;
	SceneFile *pSceneFile;
};

bool SceneFactory::checkMeshInstances(const LoadSceneFileDescription &desc, const MeshInstanceFile* instances, u32 count)
{
	bool result = true;
	for (u32 i = 0; i < count; ++i)
	{
		auto meshFile = instances[i].getMeshFile();
		auto meshSid = vx::make_sid(meshFile);

		// check for mesh
		auto itMesh = desc.sortedMeshes->find(meshSid);
		if (itMesh == desc.sortedMeshes->end())
		{
			// request load
			desc.pMissingFiles->push_back(vx::FileEntry(meshFile, vx::FileType::Mesh));

			result = false;
		}

		// check for material
		auto materialFile = instances[i].getMaterialFile();
		auto materialSid = vx::make_sid(materialFile);
		auto itMaterial = desc.sortedMaterials->find(materialSid);
		if (itMaterial == desc.sortedMaterials->end())
		{
			desc.pMissingFiles->push_back(vx::FileEntry(materialFile, vx::FileType::Material));

			result = false;
		}
	}

	return result;
}

bool SceneFactory::checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc)
{
	auto pMeshInstances = desc.pSceneFile->getMeshInstances();
	auto instanceCount = desc.pSceneFile->getNumMeshInstances();

	bool result = checkMeshInstances(desc, pMeshInstances, instanceCount);

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

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator,Scene *pScene)
{
	auto marker = scratchAllocator->getMarker();
	bool result = false;
	SceneFile sceneFile = FileFactory::load(ptr, fileSize, &result, scratchAllocator);

	if (result)
	{
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Loaded Scene File version %u", sceneFile.getVersion());

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

	scratchAllocator->clear(marker);

	return result;
}

bool SceneFactory::createFromFile(const Factory::CreateSceneDescription &desc, vx::File* file, vx::StackAllocator* scratchAllocator, Editor::Scene *pScene)
{
	bool result = false;
	SceneFile sceneFile = FileFactory::load(file, &result, scratchAllocator, nullptr);

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

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Editor::Scene* pScene)
{
	auto marker = scratchAllocator->getMarker();
	bool result = false;
	SceneFile sceneFile = FileFactory::load(ptr, fileSize, &result, scratchAllocator);

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

	scratchAllocator->clear(marker);

	return result;
}

/*bool SceneFactory::save(const Editor::Scene &scene, vx::File* file)
{
	SceneFile sceneFile;
	convert(scene, &sceneFile);

	return sceneFile.saveToFile(file);
}*/

void SceneFactory::saveToFile(const Editor::Scene &scene, vx::File* f)
{
	SceneFile sceneFile(SceneFile::getGlobalVersion());
	convert(scene, &sceneFile);

	vx::FileFactory::saveToFile(f, &sceneFile);
}

void SceneFactory::convert(const Editor::Scene &scene, SceneFile* sceneFile)
{
	ConverterEditorSceneToSceneFile::convert(scene, sceneFile);
}

void SceneFactory::deleteScene(Editor::Scene *scene)
{
	delete(scene);
}