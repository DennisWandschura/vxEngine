#pragma once

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

#include <vxEngineLib/Graphics/Surface.h>
#include <vector>

namespace Graphics
{
	enum TextureFormat : u8
	{
		RED,
		BG,
		BGR,
		BGRA,
		RG,
		RGB,
		RGBA,
		DXT1,
		DXT3,
		DXT5,
		BC7_UNORM_SRGB,
		BC7_UNORM,
		BC6H_UF16,
		BC6H_SF16
	};

	enum class TextureType : u8 { Flat, Cubemap, Volume };

	class Face : public Surface
	{
		managed_ptr<Surface[]> m_mipmaps;
		u32 m_mipmapCount;

	public:
		Face();
		Face(const Face&) = delete;
		Face(Face &&rhs);
		~Face();

		Face& operator=(const Face&) = delete;
		Face& operator=(Face &&rhs);

		void create(const vx::uint3 &dimension, u32 size, managed_ptr<u8[]> &&pixels);
		void setMipmaps(managed_ptr<Surface[]> &&mipmaps, u32 count);

		void clear();

		u32 getMipmapCount() const { return m_mipmapCount; }
		Surface* getMipmap(u32 i) { return &m_mipmaps[i]; }
		const Surface* getMipmap(u32 i) const { return &m_mipmaps[i]; }
	};

	class Texture
	{
		managed_ptr<Face[]> m_faces;
		u32 m_faceCount;
		TextureFormat m_format;
		TextureType m_type;
		u8 m_components;

	public:
		Texture();
		Texture(const Texture&) = delete;
		Texture(Texture &&rhs);
		~Texture();

		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture &&rhs);

		void create(managed_ptr<Face[]> &&faces, TextureFormat format, TextureType type, u8 components);
		void clear();

		Face& getFace(u32 i) { return m_faces[i]; }
		const Face& getFace(u32 i) const { return m_faces[i]; }

		TextureFormat getFormat() const { return m_format; }

		u32 getFaceCount() const;
		u8 getComponents() const { return m_components; }
	};
}
