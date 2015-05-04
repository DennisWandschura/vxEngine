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
#include "TextureLayer.h"

namespace Graphics
{
	TextureLayer::TextureLayer()
		:m_data(),
		m_dimension(0, 0, 0),
		m_size(0)
	{

	}

	TextureLayer::TextureLayer(TextureLayer &&rhs)
		:m_data(std::move(rhs.m_data)),
		m_dimension(rhs.m_dimension),
		m_size(rhs.m_size)
	{

	}

	TextureLayer::~TextureLayer()
	{

	}

	void TextureLayer::create(const vx::ushort3 &dim, U32 size, std::unique_ptr<U8> &&data)
	{
		m_data = std::move(data);
		m_dimension = dim;
		m_size = size;
	}

	void TextureLayer::clear()
	{
		m_data.reset();
		m_dimension = vx::ushort3(0, 0,0);
		m_size = 0;
	}

	const U8* TextureLayer::getData() const
	{
		return m_data.get();
	}

	const vx::ushort3& TextureLayer::getDim() const
	{
		return m_dimension;
	}

	U32 TextureLayer::getSize() const
	{
		return m_size;
	}
}