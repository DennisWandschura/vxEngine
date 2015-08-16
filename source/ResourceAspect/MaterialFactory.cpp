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
#include <vxResourceAspect/MaterialFactory.h>
#include <vxResourceAspect/FileEntry.h>
#include <vxEngineLib/Material.h>
#include <vxLib/Container/sorted_array.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxEngineLib/Graphics/Texture.h>

namespace MaterialFactoryCpp
{
	struct CheckTextureFileDesc
	{
		const char* filename;
		const ResourceManager<Graphics::Texture>* textureManager;
		MissingTextureFile* missingFiles;
		u32* count; 
		vx::StringID* outSid;
		bool srgb;
	};

	bool checkTextureFile(const CheckTextureFileDesc &desc)
	{
		vx::FileEntry fileEntry(desc.filename, vx::FileType::Texture);
		auto sid = fileEntry.getSid();

		bool lresult = true;

		auto ref = desc.textureManager->find(sid);
		if (ref == nullptr)
		{
			auto index = *desc.count;

			desc.missingFiles[index].fileEntry = fileEntry;
			desc.missingFiles[index].srgb = desc.srgb;

			++(*desc.count);

			lresult = false;
		}
		else
		{
			*desc.outSid = sid;
		}

		return lresult;
	}

	bool checkTextureFile(const char(&filename)[32], const vx::sorted_array<vx::StringID, const Graphics::Texture*> *textureFiles, std::vector<vx::FileEntry>* missingFiles, vx::StringID* outSid)
	{
		vx::FileEntry fileEntry(filename, vx::FileType::Texture);
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
	if (materialFile.loadFromFile(desc.fileNameWithPath))
	{
		result = true;
		// make sure all texture files are loaded
		if (!MaterialFactoryCpp::checkTextureFile(materialFile.m_albedo.data, desc.textureFiles, desc.missingFiles, &texSid[0]))
			result = false;
		if (!MaterialFactoryCpp::checkTextureFile(materialFile.m_normal.data, desc.textureFiles, desc.missingFiles, &texSid[1]))
			result = false;
		if (!MaterialFactoryCpp::checkTextureFile(materialFile.m_surface.data, desc.textureFiles, desc.missingFiles, &texSid[2]))
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

bool MaterialFactory::load(const MaterialFactoryLoadDescNew &desc)
{
	bool result = false;

	vx::StringID texSid[3];
	MaterialFile materialFile;
	if (materialFile.loadFromFile(desc.fileNameWithPath))
	{
		result = true;
		// make sure all texture files are loaded
		MaterialFactoryCpp::CheckTextureFileDesc checkDesc;
		checkDesc.filename = materialFile.m_albedo.data;
		checkDesc.textureManager = desc.m_textureManager;
		checkDesc.missingFiles = desc.missingFiles;
		checkDesc.count = desc.missingFilesCount;
		checkDesc.outSid = &texSid[0];
		checkDesc.srgb = true;

		if (!MaterialFactoryCpp::checkTextureFile(checkDesc))
			result = false;

		checkDesc.filename = materialFile.m_normal.data;
		checkDesc.outSid = &texSid[1];
		checkDesc.srgb = false;
		if (!MaterialFactoryCpp::checkTextureFile(checkDesc))
			result = false;

		checkDesc.filename = materialFile.m_surface.data;
		checkDesc.outSid = &texSid[2];
		checkDesc.srgb = false;
		if (!MaterialFactoryCpp::checkTextureFile(checkDesc))
			result = false;

		/*if (!MaterialFactoryCpp::checkTextureFile(materialFile.m_normal.data, desc.m_textureManager, desc.missingFiles, desc.missingFilesCount, &texSid[1]))
			result = false;
		if (!MaterialFactoryCpp::checkTextureFile(materialFile.m_surface.data, desc.m_textureManager, desc.missingFiles, desc.missingFilesCount, &texSid[2]))
			result = false;*/
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