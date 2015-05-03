#include "MaterialFactory.h"
#include "Material.h"
#include "FileEntry.h"
#include "TextureFile.h"
#include <vxLib/Container/sorted_array.h>
#include "enums.h"

namespace
{
	bool checkTextureFile(const char(&filename)[32], const vx::sorted_array<vx::StringID, TextureFile*> *textureFiles, std::vector<FileEntry>* missingFiles, vx::StringID* outSid)
	{
		FileEntry fileEntry(filename, FileType::Texture);
		vx::StringID sid = vx::make_sid(filename);

		bool lresult = true;
		auto it = textureFiles->find(sid);
		if (it == textureFiles->end())
		{
			missingFiles->push_back(fileEntry);
			lresult = false;
		}
		else
		{
			*outSid = sid;
		}

		return lresult;
	}
}


bool MaterialFactory::load(const MaterialFactoryLoadDescription &desc)
{
	bool result = false;

	vx::StringID texSid[3];
	MaterialFile materialFile;
	if (materialFile.loadFromFile(desc.file))
	{
		result = true;
		// make sure all texture files are loaded
		if (!checkTextureFile(materialFile.m_albedo.data, desc.textureFiles, desc.missingFiles, &texSid[0]))
			result = false;
		if (!checkTextureFile(materialFile.m_normal.data, desc.textureFiles, desc.missingFiles, &texSid[1]))
			result = false;
		if (!checkTextureFile(materialFile.m_surface.data, desc.textureFiles, desc.missingFiles, &texSid[2]))
			result = false;
	}

	if (result)
	{
		for (auto i = 0u; i < 3; ++i)
		{
			desc.material->m_textureSid[i] = texSid[i];
		}

		desc.material->setPhysx(materialFile.m_staticFriction, materialFile.m_dynamicFriction, materialFile.m_restitution);
	}

	return result;
}