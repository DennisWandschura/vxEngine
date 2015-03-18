#include "FontAtlas.h"
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

FontAtlasEntry FontAtlas::readEntry(std::ifstream &infile, U32 &id)
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

size_t FontAtlas::readEntry(const char *ptr, FontAtlasEntry &entry, U32 &id)
{
	auto p = ptr;

	p = getCharacter(p, '=');
	sscanf_s(p, "%u", &id);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.x);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.y);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.width);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.height);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.offsetX);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.offsetY);

	p = getCharacter(p, '=');
	sscanf_s(p, "%f", &entry.advanceX);

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

	U32 entryCount = 0;
	infile >> entryCount;

	assert(entryCount != 0);
	m_data.reserve(entryCount);

	for (U32 i = 0; i < entryCount; ++i)
	{
		U32 id = 0;
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

	U32 entryCount = 0;
	sscanf(ptr, "%u", &entryCount);

	assert(entryCount != 0);

	//ptr += r;
	ptr += sizeof(U32);

	m_data.reserve(entryCount);

	for (U32 i = 0; i < entryCount; ++i)
	{
		U32 id = 0;
		FontAtlasEntry entry;
		auto n = readEntry(ptr, entry, id);

		ptr += n;

		m_data.insert(id, entry);
	}

	return true;
}

const FontAtlasEntry* FontAtlas::getEntry(U32 code) const
{
	const FontAtlasEntry *pEntry = nullptr;

	auto it = m_data.find(code);
	if (it != m_data.end())
	{
		pEntry = &*it;
	}

	return pEntry;
}