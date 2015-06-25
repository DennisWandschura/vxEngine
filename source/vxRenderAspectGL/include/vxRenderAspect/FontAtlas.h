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

#include <vxLib/Container/sorted_vector.h>

struct FontAtlasEntry
{
	f32 x;
	f32 y;
	f32 width;
	f32 height;
	f32 offsetX;
	f32 offsetY;
	f32 advanceX;

	FontAtlasEntry();
};

class FontAtlas
{
	vx::sorted_vector<u32, FontAtlasEntry> m_data;

	FontAtlasEntry readEntry(std::ifstream &infile, u32 &id);
	size_t readEntry(const char *ptr, FontAtlasEntry &entry, u32 &id);

public:
	FontAtlas();
	FontAtlas(const FontAtlas&) = delete;
	FontAtlas(FontAtlas &&other);
	~FontAtlas();

	FontAtlas& operator=(const FontAtlas &rhs) = delete;
	FontAtlas& operator=(FontAtlas &&rhs);

	bool loadFromFile(const char *file);
	bool loadFromMemory(const char *data);

	const FontAtlasEntry* getEntry(u32 code) const;
};