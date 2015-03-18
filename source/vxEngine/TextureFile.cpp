#include "TextureFile.h"
#include <vxLib\stb_image.h>
#include <fstream>
#include <algorithm>
#include <vxLib/gl/gl.h>

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

void TextureFile::flipImage(U8 *data, U32 width, U32 height, U8 n)
{
	U32 width_in_bytes = width * n;
	U8 temp = 0;
	U32 half_height = height / 2;

	for (U32 row = 0; row < half_height; row++)
	{
		auto top = data + row * width_in_bytes;
		auto bottom = data + (height - row - 1) * width_in_bytes;
		for (U32 col = 0; col < width_in_bytes; col++)
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
	auto pData = std::unique_ptr<U8[]>(stbi_load(file, &x, &y, &n, 0));
	if (!pData)
		return false;

	flipImage(pData.get(), x, y, n);

	m_pMemory = std::move(pData);
	m_size.x = x;
	m_size.y = y;
	m_channels = n;
	m_mipmapLevels = 1;

	m_pData = std::make_unique<Level[]>(1);
	m_pData[0].ptr = m_pMemory.get();
	m_pData[0].sizeInBytes = x * y * n;

	return true;
}

bool TextureFile::load(const U8 *ptr, U32 size)
{
	int x, y, n;
	std::unique_ptr<U8[]> pData(stbi_load_from_memory(ptr, size, &x, &y, &n, 0));
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

	m_pData = std::make_unique<Level[]>(1);
	m_pData[0].ptr = m_pMemory.get();
	m_pData[0].sizeInBytes = x * y * n;

	return true;
}