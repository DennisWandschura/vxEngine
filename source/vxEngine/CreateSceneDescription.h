#pragma once

namespace vx
{
	template<typename K, typename T>
	class sorted_array;

	template<typename K, typename T>
	class sorted_vector;

	class Mesh;
	
	using StringID = U64;
}

class Material;
class FileEntry;

#include <vector>

namespace Factory
{
	struct CreateSceneDescription
	{
		const vx::sorted_array<vx::StringID, vx::Mesh*>* meshes;
		const vx::sorted_array<vx::StringID, Material*>* materials;
		const vx::sorted_vector<vx::StringID, std::string>* loadedFiles;
		std::vector<FileEntry>* pMissingFiles;
	};
}