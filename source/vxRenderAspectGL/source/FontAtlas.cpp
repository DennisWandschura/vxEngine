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
#include "vxRenderAspect/FontAtlas.h"
#include <fstream>
#include <assert.h>

inline const char* getCharacter(const char *str, char in)
{
	char c = *str;
	while (c != in)
	{
		c = *str;
		++str;
	}

	return str;
}

FontAtlasEntry::FontAtlasEntry()
	:x(0),
	y(0),
	width(0),
	height(0),
	offsetX(0),
	offsetY(0),
	advanceX(0)
{
}

FontAtlas::FontAtlas()
	:m_data()
{
}

FontAtlas::FontAtlas(FontAtlas &&other)
	: m_data(std::move(other.m_data))
{
}

FontAtlas::~FontAtlas()
{
}

FontAtlas& FontAtlas::operator = (FontAtlas &&rhs)
{
	if (this != &rhs)
	{
		m_data = std::move(rhs.m_data);
	}
	return *this;
}

FontAtlasEntry FontAtlas::readEntry(std::ifstream &infile, u32 &id)
{
	FontAtlasEntry entry;
	char input;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> id;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.x;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.y;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.width;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.height;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.offsetX;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.offsetY;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}
	infile >> entry.advanceX;

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}

	return entry;
}

size_t FontAtlas::readEntry(const char *ptr, FontAtlasEntry &entry, u32 &id)
{
	auto p = ptr;

	p = getCharacter(p, '=');
	sscanf(p, "%u", &id);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.x);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.y);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.width);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.height);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.offsetX);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.offsetY);

	p = getCharacter(p, '=');
	sscanf(p, "%f", &entry.advanceX);

	p = getCharacter(p, '=');

	p = getCharacter(p, '=');

	size_t result = p - ptr;

	return result;
}

bool FontAtlas::loadFromFile(const char *file)
{
	std::ifstream infile;

	infile.open(file);
	if (!infile.is_open())
		return false;

	char input;
	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}

	infile.get(input);
	while (input != '=')
	{
		infile.get(input);
	}

	u32 entryCount = 0;
	infile >> entryCount;

	assert(entryCount != 0);
	m_data.reserve(entryCount);

	for (u32 i = 0; i < entryCount; ++i)
	{
		u32 id = 0;
		auto entry = readEntry(infile, id);

		m_data.insert(id, entry);
	}

	return true;
}

bool FontAtlas::loadFromMemory(const char *data)
{
	if (data == nullptr)
		return false;

	auto ptr = data;

	char input = *ptr;
	while (input != '=')
	{
		input = *ptr;
		++ptr;
	}

	input = *ptr;
	while (input != '=')
	{
		input = *ptr;
		++ptr;
	}

	u32 entryCount = 0;
	sscanf(ptr, "%u", &entryCount);

	assert(entryCount != 0);

	//ptr += r;
	ptr += sizeof(u32);

	m_data.reserve(entryCount);

	for (u32 i = 0; i < entryCount; ++i)
	{
		u32 id = 0;
		FontAtlasEntry entry;
		auto n = readEntry(ptr, entry, id);

		ptr += n;

		m_data.insert(id, entry);
	}

	return true;
}

const FontAtlasEntry* FontAtlas::getEntry(u32 code) const
{
	const FontAtlasEntry *pEntry = nullptr;

	auto it = m_data.find(code);
	if (it != m_data.end())
	{
		pEntry = &*it;
	}

	return pEntry;
}