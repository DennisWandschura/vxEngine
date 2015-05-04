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