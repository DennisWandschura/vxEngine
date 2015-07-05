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
#include <vxEngineLib/TextureFile.h>
#include <vxLib/stb_image.h>
#include <fstream>
#include <algorithm>

TextureFile::TextureFile()
	:m_pData(nullptr),
	m_pMemory(),
	m_channels(0),
	m_settings(),
	m_mipmapLevels(0)
{

}

TextureFile::TextureFile(TextureFile &&rhs)
	:m_pData(std::move(rhs.m_pData)),
	m_pMemory(std::move(rhs.m_pMemory)),
	m_size(rhs.m_size),
	m_channels(rhs.m_channels),
	m_settings(rhs.m_settings),
	m_mipmapLevels(rhs.m_mipmapLevels)
{
	rhs.m_pData = nullptr;
}

TextureFile& TextureFile::operator = (TextureFile &&rhs)
{
	if (this != &rhs)
	{
		m_pData = std::move(rhs.m_pData);
		m_pMemory = std::move(rhs.m_pMemory);
		m_size = rhs.m_size;
		m_channels = rhs.m_channels;
		m_settings = rhs.m_settings;
		m_mipmapLevels = rhs.m_mipmapLevels;

		rhs.m_size.x = 0;
		rhs.m_size.y = 0;
		rhs.m_channels = 0;

	}
	return *this;
}

void TextureFile::flipImage(u8 *data, u32 width, u32 height, u8 n)
{
	u32 width_in_bytes = width * n;
	u8 temp = 0;
	u32 half_height = height / 2;

	for (u32 row = 0; row < half_height; row++)
	{
		auto top = data + row * width_in_bytes;
		auto bottom = data + (height - row - 1) * width_in_bytes;
		for (u32 col = 0; col < width_in_bytes; col++)
		{
			temp = *top;
			*top = *bottom;
			*bottom = temp;
			top++;
			bottom++;
		}
	}
}

bool TextureFile::loadFromFile(const char *file)
{
	int x, y, n;
	auto pData = std::unique_ptr<u8[]>(stbi_load(file, &x, &y, &n, 0));
	if (!pData)
		return false;

	flipImage(pData.get(), x, y, n);

	m_pMemory = std::move(pData);
	m_size.x = x;
	m_size.y = y;
	m_channels = n;
	m_mipmapLevels = 1;

	m_pData = vx::make_unique<Level[]>(1);
	m_pData[0].ptr = m_pMemory.get();
	m_pData[0].sizeInBytes = x * y * n;

	return true;
}

bool TextureFile::load(const u8 *ptr, u32 size)
{
	int x, y, n;
	std::unique_ptr<u8[]> pData(stbi_load_from_memory(ptr, size, &x, &y, &n, 0));
	if (!pData)
	{
		printf("TextureFile::load: failed to load from memory\n");
		return false;
	}

	flipImage(pData.get(), x, y, n);

	m_pMemory = std::move(pData);
	m_size.x = x;
	m_size.y = y;
	m_channels = n;
	m_settings.clear<s_compressed>();
	m_settings.set<s_srgb>();
	m_mipmapLevels = 1;

	m_pData = vx::make_unique<Level[]>(1);
	m_pData[0].ptr = m_pMemory.get();
	m_pData[0].sizeInBytes = x * y * n;

	return true;
}