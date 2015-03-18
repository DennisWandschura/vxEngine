#include "Font.h"

Font::Font()
	:m_texture(),
	m_atlas()
{

}

Font::Font(TextureRef &&textureRef, FontAtlas &&fontAtlas)
	: m_texture(std::move(textureRef)),
	m_atlas(std::move(fontAtlas))
{

}

Font::Font(Font &&rhs)
	: m_texture(std::move(rhs.m_texture)),
	m_atlas(std::move(rhs.m_atlas))
{
}

Font::~Font()
{
}

Font& Font::operator=(Font &&rhs)
{
	if (this != &rhs)
	{
		m_texture = std::move(rhs.m_texture);
		m_atlas = std::move(rhs.m_atlas);
	}
	return *this;
}

const FontAtlasEntry* Font::getAtlasEntry(U32 code) const
{
	return m_atlas.getEntry(code);
}