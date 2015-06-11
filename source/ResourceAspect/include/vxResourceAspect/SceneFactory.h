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

	template<typename K, typename T, typename C>
	class sorted_array;

	template<typename K, typename T, typename Cmp>
	class sorted_vector;

	struct StringID;

	class StackAllocator;
	class File;
	class FileEntry;
}

namespace Editor
{
	class Scene;
}

class Scene;
class SceneFile;
class Material;
class MeshInstanceFileOld;
class MeshInstanceFile;

namespace Factory
{
	struct CreateSceneDescription;
}

#include <vxLib/types.h>
#include <vector>

class SceneFactory
{
	struct LoadSceneFileDescription;

	static bool checkMeshInstances(const LoadSceneFileDescription &desc, const MeshInstanceFile* instances, u32 count);
	static bool checkMeshInstances(const LoadSceneFileDescription &desc, const MeshInstanceFileOld* instances, u32 count);
	static bool checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc);

public:
	static bool createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, u32 fileSize, Scene *pScene);

	static bool createFromFile(const Factory::CreateSceneDescription &desc, vx::File* file, vx::StackAllocator* allocator, Editor::Scene *pScene);
	static bool createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, u32 fileSize, Editor::Scene *pScene);

	static void convert(const Editor::Scene &scene, SceneFile* sceneFile);

	static void saveToFile(const Editor::Scene &scene, vx::File* f);

	static void deleteScene(Editor::Scene *scene);
};