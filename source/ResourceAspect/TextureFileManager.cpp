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

#include <vxResourceAspect/TextureFileManager.h>
#include <vxResourceAspect/FileEntry.h>
#include <vxEngineLib/Logfile.h>
#include <vxEngineLib/TextureFile.h>

TextureFileManager::TextureFileManager()
{

}

TextureFileManager::~TextureFileManager()
{

}

void TextureFileManager::initialize(u32 maxCount, vx::StackAllocator *pMainAllocator)
{
	m_sortedTextureFiles = vx::sorted_array<vx::StringID, TextureFile*>(maxCount, pMainAllocator);

	auto sizeInBytes = sizeof(TextureFile) * maxCount;
	m_poolTextureFile.initialize(pMainAllocator->allocate(sizeInBytes, 64), maxCount);
}

void TextureFileManager::shutdown()
{
	m_poolTextureFile.clear();
	m_poolTextureFile.release();

	m_sortedTextureFiles.cleanup();
}

TextureFile* TextureFileManager::loadTexture(const desc::LoadTextureDescription &desc, Logfile* logfile)
{
	TextureFile *pResult = nullptr;

	auto texIt = m_sortedTextureFiles.find(desc.sid);
	if (texIt == m_sortedTextureFiles.end())
	{
		TextureFile textureFile;
		if (textureFile.load(desc.fileData, desc.fileSize))
		{
			u16 index;
			auto textureFilePtr = m_poolTextureFile.createEntry(&index, std::move(textureFile));
			VX_ASSERT(textureFilePtr != nullptr);

			texIt = m_sortedTextureFiles.insert(desc.sid, textureFilePtr);
			pResult = textureFilePtr;

			*desc.status = vx::FileStatus::Loaded;
			logfile->fWriteEntry(__FILE__, __FUNCTION__, __LINE__, Logfile::Normal, false, "Loaded Texture '%s' %llu\n", desc.filename, desc.sid.value);
		}
		else
		{
			logfile->fWriteEntry(__FILE__, __FUNCTION__, __LINE__, Logfile::Error, false, "Error loading texture '%s'\n", desc.filename);
		}
	}
	else
	{
		*desc.status = vx::FileStatus::Exists;
		pResult = *texIt;
	}


	return pResult;
}

const TextureFile* TextureFileManager::getTextureFile(vx::StringID sid) const noexcept
{
	const TextureFile *p = nullptr;

	auto it = m_sortedTextureFiles.find(sid);
	if (it != m_sortedTextureFiles.end())
		p = *it;

	return p;
}

const vx::sorted_array<vx::StringID, TextureFile*>& TextureFileManager::getSortedTextureFiles() const
{
	return m_sortedTextureFiles;
}