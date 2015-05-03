#pragma once

class Scene;
class SceneFile;
class Material;
class MeshInstance;
struct Actor;

namespace vx
{
	class Mesh;
}

#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>

class ConverterSceneFileToScene
{
	struct CreateSceneMeshInstancesDesc
	{
		const SceneFile *sceneFile;
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		MeshInstance* pMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	struct CreateSceneActorsDesc
	{
		const SceneFile *sceneFile;
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes; 
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	static bool createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc);
	static bool createSceneActors(const CreateSceneActorsDesc &desc);

public:
	static bool convert(const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes, const vx::sorted_array<vx::StringID, Material*> *sortedMaterials, const SceneFile &sceneFile, Scene* scene);
};