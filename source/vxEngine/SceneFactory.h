#pragma once

#include <vxLib\types.h>

namespace vx
{
	class Mesh;

	template<typename K, typename T>
	class sorted_array;

	template<typename K, typename T>
	class sorted_vector;

	class StringID64;
	class StackAllocator;
}

class Scene;
class EditorScene;
class SceneFile;
class FileEntry;
class Material;
class File;
struct CreateSceneDescription;

#include <vector>

class SceneFactory
{
	struct LoadSceneFileDescription;

	static bool checkIfAssetsAreLoaded(const LoadSceneFileDescription &desc);

public:
	static bool createFromMemory(const CreateSceneDescription &desc, const U8* ptr, Scene *pScene);

	static bool createFromFile(const CreateSceneDescription &desc, File* file, vx::StackAllocator* allocator, EditorScene *pScene);
	static bool createFromMemory(const CreateSceneDescription &desc, const U8* ptr, EditorScene *pScene);

	static bool save(const EditorScene &scene, File* file);

	static void convert(const EditorScene &scene, SceneFile* sceneFile);
};