#pragma once

#include <vxLib/types.h>

class Material;
class FileEntry;
class TextureFile;

namespace vx
{
	class StringID64;

	template<typename K, typename T>
	class sorted_array;
}

#include <vector>

class MaterialFactory
{
	static bool checkTextureFile(const char(&filename)[32], const vx::sorted_array<vx::StringID64, TextureFile> &textureFiles, std::vector<FileEntry>* missingFiles, vx::StringID64* outSid);

public:
	static std::pair<bool, Material> load(const char *file, const vx::sorted_array<vx::StringID64, TextureFile> &textureFiles, std::vector<FileEntry>* missingFiles);
};