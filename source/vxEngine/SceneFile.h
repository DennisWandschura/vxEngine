#pragma once

struct MeshInstanceFile;
struct Light;
struct SpawnFile;
struct ActorFile;
struct Actor;

class MeshInstance;
class File;
class Material;
class Scene;
class EditorScene;

namespace vx
{
	template<typename T, typename U>
	class sorted_array;

	class Mesh;
	class StringID64;
}

#include "NavMesh.h"
#include <memory>
#include <vxLib/Container/sorted_vector.h>

class SceneFile
{
	std::unique_ptr<MeshInstanceFile[]> m_pMeshInstances;
	std::unique_ptr<Light[]> m_pLights;
	std::unique_ptr<SpawnFile[]> m_pSpawns;
	std::unique_ptr<ActorFile[]> m_pActors;
	NavMesh m_navMesh;
	U32 m_meshInstanceCount{ 0 };
	U32 m_lightCount{ 0 };
	U32 m_spawnCount{ 0 };
	U32 m_actorCount{ 0 };

	bool createSceneMeshInstances(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
		MeshInstance* pMeshInstances, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials);
	bool createSceneActors(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
		vx::sorted_vector<vx::StringID64, Actor>* actors, vx::sorted_vector<vx::StringID64, const vx::Mesh*>* sceneMeshes, vx::sorted_vector<vx::StringID64, Material*>* sceneMaterials);

public:
	SceneFile();
	~SceneFile();

	const U8* load(const U8 *ptr);
	void loadFromYAML(const char *file);

	void saveToFile(const char *file) const;
	void saveToFile(File *file) const;
	void saveToYAML(const char *file) const;

	const std::unique_ptr<MeshInstanceFile[]>& getMeshInstances() const noexcept;
	U32 getNumMeshInstances() const noexcept;

	U8 createScene(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials, Scene *pScene);
	U8 createScene(const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials, EditorScene *pScene);

	U32 getActorCount() const { return m_actorCount; }
	const ActorFile* getActors() const { return m_pActors.get(); }
};