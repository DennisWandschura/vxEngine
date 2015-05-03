#pragma once

#include <vxLib/types.h>

class Material;
class FileEntry;
class TextureFile;

namespace vx
{
	template<typename K, typename T>
	class sorted_array;

	using StringID = U64;
}

#include <vector>

struct MaterialFactoryLoadDescription
{
	const char *file;
	const vx::sorted_array<vx::StringID, TextureFile*>* textureFiles;
	std::vector<FileEntry>* missingFiles;
	Material* material;
};

class MaterialFactory
{
public:
	static bool load(const MaterialFactoryLoadDescription &desc);
};