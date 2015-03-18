#include "SceneFactory.h"
#include "SceneFile.h"
#include "Scene.h"
#include "FileEntry.h"
#include "MeshInstance.h"
#include "sorted_array.h"
#include <vxLib\Graphics\Mesh.h>
#include "Material.h"
#include "Light.h"
#include "enums.h"
#include "Actor.h"

U8 SceneFactory::loadSceneFile(const U8 *ptr, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
	std::vector<FileEntry> *pMissingFiles, SceneFile *pSceneFile)
{
	pSceneFile->load(ptr);

	auto &pMeshInstances = pSceneFile->getMeshInstances();
	auto meshInstanceCount = pSceneFile->getNumMeshInstances();

	// check if all meshes are loaded
	U8 result = 1;
	for (U32 i = 0; i < meshInstanceCount; ++i)
	{
		auto meshFile = pMeshInstances[i].getMeshFile();
		auto meshSid = vx::make_sid(meshFile);

		// check for mesh
		auto itMesh = meshes.find(meshSid);
		if (itMesh == meshes.end())
		{
			// request load
			pMissingFiles->push_back(FileEntry(meshFile, FileType::Mesh));

			result = 0;
		}

		// check for material
		auto materialFile = pMeshInstances[i].getMaterialFile();
		auto materialSid = vx::make_sid(materialFile);
		auto itMaterial = materials.find(materialSid);
		if (itMaterial == materials.end())
		{
			pMissingFiles->push_back(FileEntry(materialFile, FileType::Material));

			result = 0;
		}
	}

	auto pActors = pSceneFile->getActors();
	auto actorCount = pSceneFile->getActorCount();
	for (U32 i = 0; i < actorCount; ++i)
	{
		auto &actor = pActors[i];

		auto meshSid = vx::make_sid(actor.mesh);
		auto materialSid = vx::make_sid(actor.material);

		// check for mesh
		auto itMesh = meshes.find(meshSid);
		if (itMesh == meshes.end())
		{
			// request load
			pMissingFiles->push_back(FileEntry(actor.mesh, FileType::Mesh));

			result = 0;
		}

		auto itMaterial = materials.find(materialSid);
		if (itMaterial == materials.end())
		{
			// request load
			pMissingFiles->push_back(FileEntry(actor.material, FileType::Material));

			result = 0;
		}
	}

	return result;
}

U8 SceneFactory::load(const U8 *ptr, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
	std::vector<FileEntry> *pMissingFiles, Scene *pScene)
{
	SceneFile sceneFile;
	auto result = loadSceneFile(ptr, meshes, materials, pMissingFiles, &sceneFile);

	if (result != 0)
	{
		result = sceneFile.createScene(meshes, materials, pScene);
	}

	return result;
}

U8 SceneFactory::load(const U8 *ptr, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
	std::vector<FileEntry> *pMissingFiles, EditorScene *pScene)
{
	SceneFile sceneFile;
	auto result = loadSceneFile(ptr, meshes, materials, pMissingFiles, &sceneFile);

	if (result != 0)
	{
		result = sceneFile.createScene(meshes, materials, pScene);
	}

	return result;
}

U8 SceneFactory::save(const EditorScene *p, File* file)
{
	/*SceneFile sceneFile(*p);
	sceneFile.saveToYAML("test.yaml");
	sceneFile.saveToFile(file);

	return 1;*/
	return 0;
}