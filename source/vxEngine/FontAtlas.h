#pragma once

#include <vxLib/Container/sorted_vector.h>

struct FontAtlasEntry
{
	F32 x;
	F32 y;
	F32 width;
	F32 height;
	F32 offsetX;
	F32 offsetY;
	F32 advanceX;

	FontAtlasEntry();
};

class FontAtlas
{
	vx::sorted_vector<U32, FontAtlasEntry> m_data;

	FontAtlasEntry readEntry(std::ifstream &infile, U32 &id);
	size_t readEntry(const char *ptr, FontAtlasEntry &entry, U32 &id);

public:
	FontAtlas();
	FontAtlas(const FontAtlas&) = delete;
	FontAtlas(FontAtlas &&other);
	~FontAtlas();

	FontAtlas& operator=(const FontAtlas &rhs) = delete;
	FontAtlas& operator=(FontAtlas &&rhs);

	bool loadFromFile(const char *file);
	bool loadFromMemory(const char *data);

	const FontAtlasEntry* getEntry(U32 code) const;
};