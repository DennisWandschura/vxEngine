#pragma once

namespace vx
{
	template<typename K, typename T>
	class sorted_array;

	template<typename K, typename T>
	class sorted_vector;

	class Mesh;
	struct StringID64;
}

class Material;
class FileEntry;

#include <vector>

struct CreateSceneDescription
{
	const vx::sorted_array<vx::StringID64, vx::Mesh>* meshes;
	const vx::sorted_array<vx::StringID64, Material>* materials;
	const vx::sorted_vector<vx::StringID64, std::string>* loadedFiles;
	std::vector<FileEntry>* pMissingFiles;
};