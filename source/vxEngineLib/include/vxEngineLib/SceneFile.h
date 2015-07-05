#pragma once
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

struct MeshInstanceFile;
struct Light;
struct SpawnFile;
struct ActorFile;
struct Actor;
struct Spawn;
struct Waypoint;

class MeshInstance;
class Material;
class Scene;

template<typename T>
class Reference;

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

#include <vxEngineLib/NavMesh.h>
#include <vxLib/memory.h>
#include <vxLib/Container/sorted_vector.h>
#include <vxEngineLib/Serializable.h>
#include <vxLib/StringID.h>

struct CreateSceneDescription
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*, std::less<vx::StringID>> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>, std::less<vx::StringID>> *sortedMaterials;
	Scene *pScene;
};

struct CreateEditorSceneDescription
{
	const vx::sorted_array<vx::StringID, vx::MeshFile*, std::less<vx::StringID>> *sortedMeshes;
	const vx::sorted_array<vx::StringID, Reference<Material>, std::less<vx::StringID>> *sortedMaterials;
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
	std::unique_ptr<Waypoint[]> m_waypoints;
	NavMesh m_navMesh;
	u32 m_meshInstanceCount;
	u32 m_lightCount;
	u32 m_spawnCount;
	u32 m_actorCount;
	u32 m_waypointCount;

	bool createSceneMeshInstances(const CreateSceneMeshInstancesDesc &desc);
	bool createSceneActors(const CreateSceneActorsDesc &desc);

	bool createSceneShared(const CreateSceneShared &desc);

	const u8* loadVersion3(const u8 *ptr, const u8* last, vx::Allocator* allocator);
	const u8* loadVersion4(const u8 *ptr, const u8* last, vx::Allocator* allocator);

	u64 getCrcVersion3() const;
	u64 getCrcVersion4() const;

public:
	explicit SceneFile(u32 version);
	SceneFile(const SceneFile&) = delete;
	SceneFile(SceneFile &&rhs);
	~SceneFile();

	void swap(SceneFile &other);

	const u8* loadFromMemory(const u8 *ptr, u32 size, vx::Allocator* allocator) override;
	//void loadFromYAML(const char *file);

	void saveToFile(vx::File *file) const override;
	//void saveToYAML(const char *file) const;

	const MeshInstanceFile* getMeshInstances() const noexcept;
	u32 getNumMeshInstances() const noexcept;

	u8 createScene(const CreateSceneDescription &desc);
	u8 createScene(const CreateEditorSceneDescription &desc);

	u32 getActorCount() const;
	const ActorFile* getActors() const;

	u64 getCrc() const override;

	static u32 getGlobalVersion();
};