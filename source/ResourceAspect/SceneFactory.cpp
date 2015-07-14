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
#include <vxEngineLib/AnimationFile.h>
#include <vxEngineLib/EditorMeshInstance.h>

struct SceneFactory::LoadSceneFileDescription
{
	const vx::sorted_array<vx::StringID, Reference<vx::MeshFile>>* sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>>* sortedMaterials;
	const vx::sorted_array<vx::StringID, Reference<vx::Animation>>* sortedAnimations;
	std::vector<vx::FileEntry> *pMissingFiles;
	SceneFile *pSceneFile;
};

bool checkMeshInstanceMesh(const vx::FileEntry &meshFileEntry, const vx::sorted_array<vx::StringID, Reference<vx::MeshFile>>* sortedMeshes, std::vector<vx::FileEntry>* missingFiles)
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

bool checkMeshInstanceMaterial(const vx::FileEntry &materialFileEntry, const vx::sorted_array<vx::StringID, Reference<Material>>* sortedMaterials, std::vector<vx::FileEntry>* missingFiles)
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

bool checkMeshInstanceAnimation(const vx::FileEntry &animationFileEntry, const vx::sorted_array<vx::StringID, Reference<vx::Animation>>* sortedAnimations, std::vector<vx::FileEntry>* missingFiles)
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

bool SceneFactory::checkMeshInstances(const LoadSceneFileDescription &desc, const MeshInstanceFile* instances, u32 count)
{
	bool result = true;
	for (u32 i = 0; i < count; ++i)
	{
		auto &instance = instances[i];
		auto meshFile = instances[i].getMeshFile();
		auto meshFileEntry = vx::FileEntry(meshFile, vx::FileType::Mesh);

		// check for mesh
		if (!checkMeshInstanceMesh(meshFileEntry, desc.sortedMeshes, desc.pMissingFiles))
		{
			result = false;
		}

		// check for material
		auto materialFile = instances[i].getMaterialFile();
		auto materialFileEntry = vx::FileEntry(materialFile, vx::FileType::Material);
		if (!checkMeshInstanceMaterial(materialFileEntry, desc.sortedMaterials, desc.pMissingFiles))
		{
			result = false;
		}

		auto animationName = instances[i].getAnimation();
		if (animationName[0] != '\0')
		{
			auto animationFileEntry = vx::FileEntry(animationName, vx::FileType::Animation);
			if (!checkMeshInstanceAnimation(animationFileEntry, desc.sortedAnimations, desc.pMissingFiles))
			{
				result = false;
			}
		}

		/*auto itMaterial = desc.sortedMaterials->find(materialFileEntry.getSid());
		if (itMaterial == desc.sortedMaterials->end())
		{
			desc.pMissingFiles->push_back(materialFileEntry);
			//printf("could not find material: %s\n", materialFile);

			result = false;
		}*/
	}

	return result;
}

bool SceneFactory::checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc)
{
	auto pMeshInstances = desc.pSceneFile->getMeshInstances();
	auto instanceCount = desc.pSceneFile->getNumMeshInstances();

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

bool SceneFactory::createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Scene *pScene)
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
		loadDesc.sortedAnimations = desc.animations;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}
	else
	{
		//printf("Failed loading scene file (wrong header/crc)\n");
	}

	if (result)
	{
		result = ConverterSceneFileToScene::convert(desc.meshes, desc.materials, sceneFile, pScene);
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

bool SceneFactory::createFromFile(const Factory::CreateSceneDescription &desc, vx::File* file, vx::StackAllocator* scratchAllocator, Editor::Scene *pScene)
{
	bool result = false;
	SceneFile sceneFile = FileFactory::load(file, &result, scratchAllocator, nullptr);

	if (result)
	{
		LoadSceneFileDescription loadDesc;
		loadDesc.sortedMaterials = desc.materials;
		loadDesc.sortedMeshes = desc.meshes;
		loadDesc.sortedAnimations = desc.animations;
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
		createSceneDescriptionDesc.sortedAnimations = desc.animations;
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
		vx::verboseChannelPrintF(0, vx::debugPrint::Channel_FileAspect, "Loaded Editor::Scene File version %u", sceneFile.getVersion());

		//printf("SceneFactory::createFromMemory: create scene file\n");
		LoadSceneFileDescription loadDesc;
		loadDesc.sortedMaterials = desc.materials;
		loadDesc.sortedMeshes = desc.meshes;
		loadDesc.sortedAnimations = desc.animations;
		loadDesc.pMissingFiles = desc.pMissingFiles;
		loadDesc.pSceneFile = &sceneFile;

		result = checkIfAssetsAreLoaded(loadDesc);
	}

	if (result)
	{
		//printf("SceneFactory::createFromMemory: try to create scene\n");
		CreateEditorSceneDescription createSceneDescriptionDesc;
		createSceneDescriptionDesc.pScene = pScene;
		createSceneDescriptionDesc.sortedMaterials = desc.materials;
		createSceneDescriptionDesc.sortedMeshes = desc.meshes;
		createSceneDescriptionDesc.sortedAnimations = desc.animations;
		createSceneDescriptionDesc.loadedFiles = desc.loadedFiles;
		result = sceneFile.createScene(createSceneDescriptionDesc);
	}
	else
	{
		//printf("SceneFactory::createFromMemory: error, missing assets\n");
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

	auto sceneFileInstanceCount = sceneFile.getNumMeshInstances();
	auto sceneFileInstances = sceneFile.getMeshInstances();
	for (u32 i = 0; i < sceneFileInstanceCount; ++i)
	{
		auto str = sceneFileInstances[i].getAnimation();
		if (str[0] != '\0')
		{
			printf("SceneFile Animation: %s\n", str);
		}
	}

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