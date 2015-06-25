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

class TextureFile;

#include <vxGL/Texture.h>
#include <vxLib/memory.h>
#include <vxLib\Container\sorted_vector.h>
#include <vxEngineLib/TextureRef.h>

class TextureManager
{
	static const u16 s_invalid{ 0xffff };
	struct TextureCmp
	{
		vx::ushort3 m_size;
		u8 miplevels;
		vx::gl::TextureType type;
		vx::gl::TextureFormat format;

		friend bool operator<(const TextureCmp &lhs, const TextureCmp &rhs)
		{
			if (lhs.type < rhs.type)
				return true;
			else if (lhs.type == rhs.type)
			{
				if (lhs.type == vx::gl::TextureType::Texture_2D_Array)
				{
					return (lhs.format < rhs.format) ||
						(lhs.format == rhs.format && lhs.m_size.x < rhs.m_size.x) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y < rhs.m_size.y) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y == rhs.m_size.y && lhs.miplevels < rhs.miplevels);
				}
				else
				{
					return (lhs.format < rhs.format) ||
						(lhs.format == rhs.format && lhs.m_size.x < rhs.m_size.x) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y < rhs.m_size.y) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y == rhs.m_size.y && lhs.m_size.z < rhs.m_size.z) ||
						(lhs.format == rhs.format && lhs.m_size.x == rhs.m_size.x && lhs.m_size.y == rhs.m_size.y && lhs.m_size.z == rhs.m_size.z && lhs.miplevels < rhs.miplevels);
				}
			}
			else
			{
				return false;
			}
		}
	};

	struct TextureWrapper
	{
		vx::gl::Texture m_texture;
		u16 m_firstFreeSlice;
		u16 m_freeSlices;
		std::unique_ptr<u16[]> m_pSlices;

		TextureWrapper();
		TextureWrapper(TextureWrapper &&rhs);

		TextureWrapper& operator=(TextureWrapper &&rhs);
	};

	struct TextureBucket
	{
		std::unique_ptr<TextureWrapper[]> m_pTextures;
		u32 m_size;
		u32 m_capacity;

		TextureBucket();
		TextureBucket(TextureBucket &&rhs);

		TextureBucket& operator=(TextureBucket &&rhs);
	};

	vx::sorted_vector<TextureCmp, TextureBucket> m_textureBuckets;
	vx::sorted_vector<u32, TextureWrapper*> m_wrappers;

	TextureBucket* findBucket(const vx::ushort3 &textureSize, u8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format);
	TextureRef createTexture2DSlice(TextureBucket *pBucket, const vx::uint2 &size, vx::gl::DataType dataType, const void *pData);

public:
	TextureManager();
	void reserveBuckets(u32 n);

	void createBucket(u32 bucketSize, const vx::ushort3 &textureSize, u8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format);

	u64 createTexture(const vx::ushort3 &size, u8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format);

	// texture files are always loaded into 2d arrays
	TextureRef load(const TextureFile &f, u8 mipLevels, u8 srgb);

	TextureRef load(const vx::ushort3 &size, u8 miplevels, vx::gl::TextureType type, vx::gl::TextureFormat format, vx::gl::DataType dataType = (vx::gl::DataType)0, const void *ptr = nullptr);

	u64 getTextureHandle(const TextureRef &ref);

	void release(TextureRef &&ref);
};