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

namespace vx
{
	class Mesh;

	template<typename K, typename T>
	class sorted_array;

	template<typename K, typename T>
	class sorted_vector;

	class StackAllocator;

	struct StringID;
}

class Scene;
class EditorScene;
class SceneFile;
class FileEntry;
class Material;
class File;

namespace Factory
{
	struct CreateSceneDescription;
}

#include <vxLib/types.h>
#include <vector>

class SceneFactory
{
	struct LoadSceneFileDescription;

	static bool checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc);

public:
	static bool createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, Scene *pScene);

	static bool createFromFile(const Factory::CreateSceneDescription &desc, File* file, vx::StackAllocator* allocator, EditorScene *pScene);
	static bool createFromMemory(const Factory::CreateSceneDescription &desc, const u8* ptr, EditorScene *pScene);

	static bool save(const EditorScene &scene, File* file);

	static void convert(const EditorScene &scene, SceneFile* sceneFile);
};