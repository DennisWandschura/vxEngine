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

namespace vx
{
	class Mesh;
	class MeshFile;
	class StackAllocator;
	class File;
	class FileEntry;

	struct Animation;
	struct StringID;

	template<typename K, typename T, typename Cmp>
	class sorted_vector;
}

namespace Editor
{
	class Scene;
}

template<typename T>
class Reference;

class Scene;
class SceneFile;
class Material;
class MeshInstanceFile;
class ResourceAspect;

template<typename T>
class ResourceManager;

#include <vxLib/types.h>
#include <vector>
#include <vxLib/Container/sorted_array.h>

namespace Factory
{
	struct CreateSceneDesc
	{
		const vx::sorted_array<vx::StringID, vx::MeshFile*>* meshes;
		const vx::sorted_array<vx::StringID, Material*>* materials;
		const vx::sorted_array<vx::StringID, vx::Animation*>* animations;
		const vx::sorted_vector<vx::StringID, std::string>* loadedFiles;
		std::vector<vx::FileEntry>* pMissingFiles;
	};

	struct CreateSceneDescNew
	{
		ResourceManager<vx::MeshFile>* meshManager;
		ResourceManager<Material>* materialManager;
		ResourceManager<vx::Animation>* animationManager;
		std::vector<vx::FileEntry>* missingFiles;
	};
}

class SceneFactory
{
	struct LoadSceneFileDescription;

	static bool checkMeshInstances(const LoadSceneFileDescription &desc, const MeshInstanceFile* instances, u32 count);
	static bool checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc);

public:
	static bool createFromMemory(const Factory::CreateSceneDesc &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Scene *pScene);

	static bool createFromFile(const Factory::CreateSceneDesc &desc, vx::File* file, vx::StackAllocator* scratchAllocator, Editor::Scene *pScene);
	static bool createFromMemory(const Factory::CreateSceneDesc &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Editor::Scene *pScene);

	static bool createFromMemory(const Factory::CreateSceneDescNew &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Scene *pScene);
	static bool createFromMemory(const Factory::CreateSceneDescNew &desc, const u8* ptr, u32 fileSize, vx::StackAllocator* scratchAllocator, Editor::Scene *pScene);

	static void convert(const Editor::Scene &scene, SceneFile* sceneFile);

	static void saveToFile(const Editor::Scene &scene, vx::File* f);

	static void deleteScene(Editor::Scene *scene);
};