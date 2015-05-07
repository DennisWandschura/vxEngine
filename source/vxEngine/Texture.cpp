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
#include "Texture.h"

namespace Graphics
{
	Texture::Texture()
		:TextureLayer(),
		m_mipmaps(),
		m_mipmapCount()
	{

	}

	void Texture::create(const vx::ushort3 &dim, u32 size, std::unique_ptr<u8> &&data, u32 mipmapCount)
	{
		TextureLayer::create(dim, size, std::move(data));

		if (mipmapCount != 0)
		{
			m_mipmaps = std::make_unique<TextureLayer[]>(mipmapCount);
			m_mipmapCount = mipmapCount;
		}
	}

	TextureLayer& Texture::getMipmap(u32 i)
	{
		return m_mipmaps[i];
	}

	const TextureLayer& Texture::getMipmap(u32 i) const
	{
		return m_mipmaps[i];
	}
}