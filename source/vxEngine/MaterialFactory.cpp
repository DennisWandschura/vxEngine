#include "MaterialFactory.h"
#include "Material.h"
#include "FileEntry.h"
#include "TextureFile.h"
#include "sorted_array.h"
#include "enums.h"

bool MaterialFactory::checkTextureFile(const char(&filename)[32], const vx::sorted_array<vx::StringID64, TextureFile> &textureFiles, std::vector<FileEntry> &missingFiles, vx::StringID64 &outSid)
{
	FileEntry fileEntry(filename, FileType::Texture);
	vx::StringID64 sid = vx::make_sid(filename);

	bool lresult = true;
	auto it = textureFiles.find(sid);
	if (it == textureFiles.end())
	{
		missingFiles.push_back(fileEntry);
		lresult = false;
	}
	else
	{
		outSid = sid;
	}

	return lresult;
}

std::pair<bool, Material> MaterialFactory::load(const char *file, vx::sorted_array<vx::StringID64, TextureFile> &textureFiles, std::vector<FileEntry> &missingFiles)
{
	std::pair<bool, Material> result;
	result.first = false;

	vx::StringID64 texSid[3];
	MaterialFile materialFile;
	if (materialFile.loadFromFile(file))
	{
		result.first = true;
		// make sure all texture files are loaded
		if (!checkTextureFile(materialFile.m_albedo.data, textureFiles, missingFiles, texSid[0]))
			result.first = false;
		if (!checkTextureFile(materialFile.m_normal.data, textureFiles, missingFiles, texSid[1]))
			result.first = false;
		if (!checkTextureFile(materialFile.m_surface.data, textureFiles, missingFiles, texSid[2]))
			result.first = false;
	}

	if (result.first)
	{
		for (auto i = 0u; i < 3; ++i)
		{
			result.second.m_textureSid[i] = texSid[i];
		}

		result.second.setPhysx(materialFile.m_staticFriction, materialFile.m_dynamicFriction, materialFile.m_restitution);
	}

	return std::move(result);
}