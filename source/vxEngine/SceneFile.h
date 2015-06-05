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
class Material;
class Scene;

namespace Editor
{
	class Scene;
}

namespace vx
{
	template<typename T, typename U, typename C>
	class sorted_array;

	class MeshFile;
	class File;
}

#include "NavMesh.h"
#include <vxLib/memory.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxEngineLib/Serializable.h>
#include <vxLib/StringID.h>

struct CreateSceneDescription
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*, std::less<vx::StringID>> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*, std::less<vx::StringID>> *sortedMaterials;
	Scene *pScene;
};

struct CreateEditorSceneDescription
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*, std::less<vx::StringID>> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Material*, std::less<vx::StringID>> *sortedMaterials;
	const vx::sorted_vector<vx::StringID, std::string> *loadedFiles;
	Editor::Scene *pScene;
};

class SceneFile : public vx::Serializable
{
	friend class ConverterSceneFileToScene;
	friend class ConverterEditorSceneToSceneFile;

	struct CreateSceneMeshInstancesDesc;
	struct CreateSceneActorsDesc;
	struct CreateSceneShared;

	std::unique_ptr<MeshInstanceFile[]> m_pMeshInstances;
	std::unique_ptr<Light[]> m_pLights;
	std::unique_ptr<SpawnFile[]> m_pSpawns;
	std::unique_ptr<ActorFile[]> m_pActors;
	NavMesh m_navMesh;
	u32 m_meshInstanceCount;
	u32 m_lightCount;
	u32 m_spawnCount;
	u32 m_actorCount;

	bool createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc);
	bool createSceneActors(const CreateSceneActorsDesc &desc);

	bool createSceneShared(const CreateSceneShared &desc);

public:
	SceneFile();
	~SceneFile();

	const u8* loadFromMemory(const u8 *ptr, u32 version, vx::Allocator* allocator) override;
	//void loadFromYAML(const char *file);

	bool saveToFile(const char *file) const;
	bool saveToFile(vx::File *file) const;
	//void saveToYAML(const char *file) const;

	const std::unique_ptr<MeshInstanceFile[]>& getMeshInstances() const noexcept;
	u32 getNumMeshInstances() const noexcept;

	u8 createScene(const CreateSceneDescription &desc);
	u8 createScene(const CreateEditorSceneDescription &desc);

	u32 getActorCount() const { return m_actorCount; }
	const ActorFile* getActors() const { return m_pActors.get(); }

	u64 getCrc() const override;

	u32 getVersion() const override;
};