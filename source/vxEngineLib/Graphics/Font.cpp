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
#include <vxEngineLib/Graphics/font.h>

namespace Graphics
{
	Font::Font()
		:m_textureSlice(0),
		m_textureDim(0),
		m_atlas()
	{

	}

	Font::Font(u32 textureSlice, u32 dim, FontAtlas &&fontAtlas)
		: m_textureSlice(textureSlice),
		m_textureDim(dim),
		m_atlas(std::move(fontAtlas))
	{

	}

	Font::Font(Font &&rhs)
		: m_textureSlice(rhs.m_textureSlice),
		m_textureDim(rhs.m_textureDim),
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
			std::swap(m_textureSlice, rhs.m_textureSlice);
			std::swap(m_textureDim, rhs.m_textureDim);
			m_atlas = std::move(rhs.m_atlas);
		}
		return *this;
	}

	const FontAtlasEntry* Font::getAtlasEntry(u32 code) const
	{
		return m_atlas.getEntry(code);
	}
}