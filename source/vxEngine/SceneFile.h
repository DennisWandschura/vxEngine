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
#pragma once

struct MeshInstanceFile;
struct Light;
struct SpawnFile;
struct ActorFile;
struct Actor;
struct Spawn;

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
}

#include "NavMesh.h"
#include <memory>
#include <vxLib/Container/sorted_vector.h>
#include "Serializable.h"
#include <vxLib/StringID.h>

struct CreateSceneDescription
{
	const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
	Scene *pScene;
};

struct CreateEditorSceneDescription
{
	const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
	const vx::sorted_vector<vx::StringID, std::string> *loadedFiles;
	EditorScene *pScene;
};

class SceneFile : public Serializable
{
	friend class ConverterSceneFileToScene;
	friend class ConverterEditorSceneToSceneFile;

	struct CreateSceneMeshInstancesDesc
	{
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		MeshInstance* pMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	struct CreateSceneActorsDesc
	{
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
	};

	struct CreateSceneShared
	{
		const vx::sorted_array<vx::StringID, vx::Mesh*> *sortedMeshes;
		const vx::sorted_array<vx::StringID, Material*> *sortedMaterials;
		MeshInstance* pMeshInstances;
		vx::sorted_vector<vx::StringID, const vx::Mesh*>* sceneMeshes;
		vx::sorted_vector<vx::StringID, Material*>* sceneMaterials;
		vx::sorted_vector<vx::StringID, Actor>* sceneActors;
		Spawn* sceneSpawns;
		U32* vertexCount;
		U32* indexCount;
	};

	std::unique_ptr<MeshInstanceFile[]> m_pMeshInstances;
	std::unique_ptr<Light[]> m_pLights;
	std::unique_ptr<SpawnFile[]> m_pSpawns;
	std::unique_ptr<ActorFile[]> m_pActors;
	NavMesh m_navMesh;
	U32 m_meshInstanceCount;
	U32 m_lightCount;
	U32 m_spawnCount;
	U32 m_actorCount;

	bool createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc);
	bool createSceneActors(const CreateSceneActorsDesc &desc);

	bool createSceneShared(const CreateSceneShared &desc);

public:
	SceneFile();
	~SceneFile();

	const U8* loadFromMemory(const U8 *ptr, U32 version);
	//void loadFromYAML(const char *file);

	bool saveToFile(const char *file) const;
	bool saveToFile(File *file) const;
	//void saveToYAML(const char *file) const;

	const std::unique_ptr<MeshInstanceFile[]>& getMeshInstances() const noexcept;
	U32 getNumMeshInstances() const noexcept;

	U8 createScene(const CreateSceneDescription &desc);
	U8 createScene(const CreateEditorSceneDescription &desc);

	U32 getActorCount() const { return m_actorCount; }
	const ActorFile* getActors() const { return m_pActors.get(); }

	U64 getCrc() const;

	U32 getVersion() const;
};