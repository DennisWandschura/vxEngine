#pragma once

#include <memory>
#include <vxLib\math\Vector.h>
#include <vxLib/Container/bitset.h>

class TextureFile
{
	struct Level
	{
		U8* ptr;
		U64 sizeInBytes;
	};

	static const U8 s_compressed = 0;
	static const U8 s_srgb = 1;

	std::unique_ptr<Level[]> m_pData;
	std::unique_ptr<U8[]> m_pMemory;
	vx::ushort2 m_size{0, 0};
	U8 m_channels;
	vx::bitset<8> m_settings;
	U8 m_mipmapLevels;

	void flipImage(U8 *data, U32 width, U32 height, U8 n);

public:
	TextureFile();
	TextureFile(const TextureFile&) = delete;
	TextureFile(TextureFile &&rhs);

	TextureFile& operator=(const TextureFile &) = delete;
	TextureFile& operator=(TextureFile &&rhs);

	bool loadFromFile(const char *file);
	bool load(const U8 *ptr, U32 size);
	//bool loadKTX(const U8 *ptr);

	const U8* get(U8 level) const { return m_pData[level].ptr; }
	const vx::ushort2& getSize() const { return m_size; }
	U8 getChannels() const { return m_channels; }
	U8 isCompressed() const { return m_settings.get<s_compressed>(); }
	U8 isSrgb() const { return m_settings.get<s_srgb>(); }
	U64 getSizeOfLevel(U8 level) const { return m_pData[level].sizeInBytes; }
};