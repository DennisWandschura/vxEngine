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
#pragma once

#include <vxLib/memory.h>
#include <vxLib\math\Vector.h>
#include <vxLib/Container/bitset.h>

class TextureFile
{
	struct Level
	{
		u8* ptr;
		u64 sizeInBytes;
	};

	static const u8 s_compressed = 0;
	static const u8 s_srgb = 1;

	std::unique_ptr<Level[]> m_pData;
	std::unique_ptr<u8[]> m_pMemory;
	vx::ushort2 m_size{0, 0};
	u8 m_channels;
	vx::bitset<8> m_settings;
	u8 m_mipmapLevels;

	void flipImage(u8 *data, u32 width, u32 height, u8 n);

public:
	TextureFile();
	TextureFile(const TextureFile&) = delete;
	TextureFile(TextureFile &&rhs);

	TextureFile& operator=(const TextureFile &) = delete;
	TextureFile& operator=(TextureFile &&rhs);

	bool loadFromFile(const char *file);
	bool load(const u8 *ptr, u32 size);
	//bool loadKTX(const u8 *ptr);

	const u8* get(u8 level) const { return m_pData[level].ptr; }
	const vx::ushort2& getSize() const { return m_size; }
	u8 getChannels() const { return m_channels; }
	u8 isCompressed() const { return m_settings.get<s_compressed>(); }
	u8 isSrgb() const { return m_settings.get<s_srgb>(); }
	u64 getSizeOfLevel(u8 level) const { return m_pData[level].sizeInBytes; }
};