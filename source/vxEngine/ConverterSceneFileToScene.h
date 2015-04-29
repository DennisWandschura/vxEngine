#pragma once

class Scene;
class SceneFile;
class Material;
class MeshInstance;
struct Actor;

namespace vx
{
	class Mesh;
	struct StringID64;
}

#include <vxLib/Container/sorted_array.h>

class ConverterSceneFileToScene
{
	static bool createSceneMeshInstances(const SceneFile &sceneFile, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes,
		const vx::sorted_array<vx::StringID64, Material> &materials,
		MeshInstance* pMeshInstances, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, 
		vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials);

	static bool createSceneActors(const SceneFile &sceneFile, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
		vx::sorted_vector<vx::StringID64, Actor>* actors, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials);

public:
	static bool convert(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials, const SceneFile &sceneFile, Scene* scene);
};