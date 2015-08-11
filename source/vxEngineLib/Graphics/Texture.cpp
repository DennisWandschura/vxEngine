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

#include <vxEngineLib/Graphics/Texture.h>

namespace Graphics
{
	Face::Face()
		:Surface(),
		m_mipmaps(),
		m_mipmapCount(0)
	{
	}

	Face::Face(Face &&rhs)
		: Graphics::Surface(std::move(rhs)),
		m_mipmaps(std::move(rhs.m_mipmaps)),
		m_mipmapCount(rhs.m_mipmapCount)
	{
		rhs.m_mipmapCount = 0;
	}

	Face::~Face()
	{
		clear();
	}

	Face& Face::operator = (Face &&rhs)
	{
		if (this != &rhs)
		{
			Graphics::Surface::operator=(std::move(rhs));
			m_mipmaps.swap(rhs.m_mipmaps);
			std::swap(m_mipmapCount, rhs.m_mipmapCount);
		}
		return *this;
	}

	void Face::create(const vx::uint3 &dimension, u32 size, managed_ptr<u8[]> &&pixels)
	{
		Graphics::Surface::create(dimension, size, std::move(pixels));

		m_mipmapCount = 0;
	}

	void Face::setMipmaps(managed_ptr<Surface[]> &&mipmaps, u32 count)
	{
		m_mipmaps = std::move(mipmaps);
		m_mipmapCount = count;
	}

	void Face::clear()
	{
		Graphics::Surface::clear();

		for (u32 i = 0;i < m_mipmapCount; ++i)
		{
			m_mipmaps[i].clear();
		}

		m_mipmaps.clear();
		m_mipmapCount = 0;
	}

	Texture::Texture()
		:m_faces(),
		m_faceCount(0),
		m_format(),
		m_type(),
		m_components(0)
	{
	}

	Texture::Texture(Texture &&rhs)
		:m_faces(std::move(rhs.m_faces)),
		m_faceCount(rhs.m_faceCount),
		m_format(rhs.m_format),
		m_type(rhs.m_type),
		m_components(rhs.m_components)
	{
		rhs.m_faceCount = 0;
		rhs.m_components = 0;
	}

	Texture::~Texture()
	{
		clear();
	}

	Texture& Texture::operator=(Texture &&rhs)
	{
		if (this != &rhs)
		{
			m_faces.swap(rhs.m_faces);
			std::swap(m_faceCount, rhs.m_faceCount);
			std::swap(m_format, rhs.m_format);
			std::swap(m_type, rhs.m_type);
			std::swap(m_components, rhs.m_components);
		}

		return *this;
	}

	void Texture::create(managed_ptr<Face[]> &&faces, TextureFormat format, TextureType type, u8 components)
	{
		clear();

		m_faces = std::move(faces);
		m_faceCount = 1;
		m_format = format;
		m_type = type;
		m_components = components;

		if (type == TextureType::Cubemap)
			m_faceCount = 6;
	}

	void Texture::clear()
	{
		for (u32 i = 0;i < m_faceCount; ++i)
		{
			m_faces[i].clear();
		}
		m_faces.clear();
		m_faceCount = 0;
		m_components = 0;
	}
}