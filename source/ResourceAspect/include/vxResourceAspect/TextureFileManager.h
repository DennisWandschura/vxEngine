#pragma once

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

#include <vxLib/types.h>

class TextureFile;
class Logfile;

namespace vx
{
	enum class FileStatus : u8;
}

#include <vxEngineLib/Pool.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>

namespace desc
{
	struct LoadTextureDescription
	{
		const char *filename;
		vx::StringID sid;
		vx::FileStatus* status;
		const u8 *fileData;
		u32 fileSize;
	};
}

class TextureFileManager
{
	vx::sorted_array<vx::StringID, TextureFile*> m_sortedTextureFiles;
	vx::Pool<TextureFile> m_poolTextureFile;

public:
	TextureFileManager();
	~TextureFileManager();

	void initialize(u32 maxCount, vx::StackAllocator *pMainAllocator);
	void shutdown();

	TextureFile* loadTexture(const desc::LoadTextureDescription &desc, Logfile* logfile);

	const TextureFile* getTextureFile(vx::StringID sid) const noexcept;
	const vx::sorted_array<vx::StringID, TextureFile*>& getSortedTextureFiles() const;
};