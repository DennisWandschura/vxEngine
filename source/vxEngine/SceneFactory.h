#pragma once

#include <vxLib\types.h>

namespace vx
{
	class Mesh;

	template<typename K, typename T>
	class sorted_array;

	class StringID64;
}

class Scene;
class EditorScene;
class SceneFile;
class FileEntry;
class Material;
class File;

#include <vector>

class SceneFactory
{
	static U8 loadSceneFile(const U8 *ptr, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
		std::vector<FileEntry> *pMissingFiles, SceneFile *pSceneFile);

public:
	/* 
		tries to load a scene file.
		after loading the file itself it checks if all required meshes and materials are present in the provided maps.
		if a required mesh or material is not found,
		the filename is inserted into fileEntries

		returns 1 on success and 0 on failure.
	*/
	static U8 load(const U8 *ptr, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
		std::vector<FileEntry> *pMissingFiles, Scene *pScene);

	static U8 load(const U8 *ptr, const vx::sorted_array<vx::StringID64, vx::Mesh> &meshes, const vx::sorted_array<vx::StringID64, Material> &materials,
		std::vector<FileEntry> *pMissingFiles, EditorScene *pScene);

	static U8 save(const EditorScene *p, File* file);
};