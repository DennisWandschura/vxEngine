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

#include <vxEngineLib/Graphics/TextureFactory.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/Graphics/dds.h>
#include <vxEngineLib/Graphics/Texture.h>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/stb_image.h>
#include <vxEngineLib/memcpy.h>
#include <vxEngineLib/ArrayAllocator.h>
#include <algorithm>

namespace Graphics
{
	struct DXTColBlock
	{
		u16 col0;
		u16 col1;

		u8 row[4];
	};

	struct DXT3AlphaBlock
	{
		u16 row[4];
	};

	struct DXT5AlphaBlock
	{
		u8 alpha0;
		u8 alpha1;

		u8 row[6];
	};

	void swap_endian(void *val)
	{
#ifdef MACOS
		u32 *ival = (u32 *)val;

		*ival = ((*ival >> 24) & 0x000000ff) |
			((*ival >> 8) & 0x0000ff00) |
			((*ival << 8) & 0x00ff0000) |
			((*ival << 24) & 0xff000000);
#endif
	}

	u32 clamp_size(u32 size)
	{
		if (size <= 0)
			size = 1;

		return size;
	}

	u32 size_dxtc(u32 width, u32 height, u8, TextureFormat format)
	{
		return ((width + 3) / 4) * ((height + 3) / 4) *
			(format == TextureFormat::DXT1 ? 8 : 16);
	}

	///////////////////////////////////////////////////////////////////////////////
	// calculates size of uncompressed RGB texture in bytes
	u32 size_rgb(u32 width, u32 height, u8 compoments, TextureFormat)
	{
		return width * height * compoments;
	}

	u32 size_bc(u32 width, u32 height, u8, TextureFormat)
	{
		return ((width + 3) / 4) * ((height + 3) / 4) * 16;
	}

	void swap(void *byte1, void *byte2, u32 size)
	{
		u8 *tmp = new u8[size];

		memcpy(tmp, byte1, size);
		memcpy(byte1, byte2, size);
		memcpy(byte2, tmp, size);

		delete[] tmp;
	}

	///////////////////////////////////////////////////////////////////////////////
	// flip a DXT1 color block
	void flip_blocks_dxtc1(DXTColBlock *line, u32 numBlocks)
	{
		DXTColBlock *curblock = line;

		for (u32 i = 0; i < numBlocks; i++)
		{
			swap(&curblock->row[0], &curblock->row[3], sizeof(u8));
			swap(&curblock->row[1], &curblock->row[2], sizeof(u8));

			curblock++;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// flip a DXT3 color block
	void flip_blocks_dxtc3(DXTColBlock *line, u32 numBlocks)
	{
		DXTColBlock *curblock = line;
		DXT3AlphaBlock *alphablock;

		for (u32 i = 0; i < numBlocks; i++)
		{
			alphablock = (DXT3AlphaBlock*)curblock;

			swap(&alphablock->row[0], &alphablock->row[3], sizeof(u16));
			swap(&alphablock->row[1], &alphablock->row[2], sizeof(u16));

			curblock++;

			swap(&curblock->row[0], &curblock->row[3], sizeof(u8));
			swap(&curblock->row[1], &curblock->row[2], sizeof(u8));

			curblock++;
		}
	}

	///////////////////////////////////////////////////////////////////////////////
	// flip a DXT5 alpha block
	void flip_dxt5_alpha(DXT5AlphaBlock *block)
	{
		u8 gBits[4][4];

		const unsigned long mask = 0x00000007;          // bits = 00 00 01 11
		unsigned long bits = 0;
		memcpy(&bits, &block->row[0], sizeof(u8) * 3);

		gBits[0][0] = (u8)(bits & mask);
		bits >>= 3;
		gBits[0][1] = (u8)(bits & mask);
		bits >>= 3;
		gBits[0][2] = (u8)(bits & mask);
		bits >>= 3;
		gBits[0][3] = (u8)(bits & mask);
		bits >>= 3;
		gBits[1][0] = (u8)(bits & mask);
		bits >>= 3;
		gBits[1][1] = (u8)(bits & mask);
		bits >>= 3;
		gBits[1][2] = (u8)(bits & mask);
		bits >>= 3;
		gBits[1][3] = (u8)(bits & mask);

		bits = 0;
		memcpy(&bits, &block->row[3], sizeof(u8) * 3);

		gBits[2][0] = (u8)(bits & mask);
		bits >>= 3;
		gBits[2][1] = (u8)(bits & mask);
		bits >>= 3;
		gBits[2][2] = (u8)(bits & mask);
		bits >>= 3;
		gBits[2][3] = (u8)(bits & mask);
		bits >>= 3;
		gBits[3][0] = (u8)(bits & mask);
		bits >>= 3;
		gBits[3][1] = (u8)(bits & mask);
		bits >>= 3;
		gBits[3][2] = (u8)(bits & mask);
		bits >>= 3;
		gBits[3][3] = (u8)(bits & mask);

		unsigned long *pBits = ((unsigned long*)&(block->row[0]));

		*pBits = *pBits | (gBits[3][0] << 0);
		*pBits = *pBits | (gBits[3][1] << 3);
		*pBits = *pBits | (gBits[3][2] << 6);
		*pBits = *pBits | (gBits[3][3] << 9);

		*pBits = *pBits | (gBits[2][0] << 12);
		*pBits = *pBits | (gBits[2][1] << 15);
		*pBits = *pBits | (gBits[2][2] << 18);
		*pBits = *pBits | (gBits[2][3] << 21);

		pBits = ((unsigned long*)&(block->row[3]));

#ifdef MACOS
		*pBits &= 0x000000ff;
#else
		*pBits &= 0xff000000;
#endif

		*pBits = *pBits | (gBits[1][0] << 0);
		*pBits = *pBits | (gBits[1][1] << 3);
		*pBits = *pBits | (gBits[1][2] << 6);
		*pBits = *pBits | (gBits[1][3] << 9);

		*pBits = *pBits | (gBits[0][0] << 12);
		*pBits = *pBits | (gBits[0][1] << 15);
		*pBits = *pBits | (gBits[0][2] << 18);
		*pBits = *pBits | (gBits[0][3] << 21);
	}

	///////////////////////////////////////////////////////////////////////////////
	// flip a DXT5 color block
	void flip_blocks_dxtc5(DXTColBlock *line, u32 numBlocks)
	{
		DXTColBlock *curblock = line;
		DXT5AlphaBlock *alphablock;

		for (u32 i = 0; i < numBlocks; i++)
		{
			alphablock = (DXT5AlphaBlock*)curblock;

			flip_dxt5_alpha(alphablock);

			curblock++;

			swap(&curblock->row[0], &curblock->row[3], sizeof(u8));
			swap(&curblock->row[1], &curblock->row[2], sizeof(u8));

			curblock++;
		}
	}

	inline bool isCompressed(TextureFormat format)
	{
		if ((format == TextureFormat::DXT1) ||
			(format == TextureFormat::DXT3) ||
			(format == TextureFormat::DXT5) ||
			(format == TextureFormat::BC6H_SF16) ||
			(format == TextureFormat::BC6H_UF16) ||
			(format == TextureFormat::BC7_UNORM) ||
			(format == TextureFormat::BC7_UNORM_SRGB))
			return true;
		else
			return false;
	}

	bool isDxt(TextureFormat format)
	{
		return ((format == TextureFormat::DXT1) ||
			(format == TextureFormat::DXT3) ||
			(format == TextureFormat::DXT5));
	}

	void flip(Surface* surface, TextureFormat format)
	{
		auto dim = surface->getDimension();

		if (!isCompressed(format))
		{
			assert(dim.z > 0);

			u32 imagesize = surface->getSize() / dim.z;
			auto linesize = imagesize / dim.y;

			for (u32 n = 0; n < dim.z; n++)
			{
				auto offset = imagesize*n;
				u8 *top = (u8*)surface + offset;
				u8 *bottom = top + (imagesize - linesize);

				for (u32 i = 0; i < (dim.y >> 1); i++)
				{
					swap(bottom, top, linesize);

					top += linesize;
					bottom -= linesize;
				}
			}
		}
		else
		{
			void (*flipblocks)(DXTColBlock*, u32);
			u32 xblocks = dim.x / 4;
			u32 yblocks = dim.y / 4;
			u32 blocksize;

			switch (format)
			{
			case TextureFormat::DXT1:
				blocksize = 8;
				flipblocks = &flip_blocks_dxtc1;
				break;
			case TextureFormat::DXT3:
				blocksize = 16;
				flipblocks = &flip_blocks_dxtc3;
				break;
			case TextureFormat::DXT5:
				blocksize = 16;
				flipblocks = &flip_blocks_dxtc5;
				break;
			default:
				return;
			}

			auto linesize = xblocks * blocksize;

			DXTColBlock *top;
			DXTColBlock *bottom;

			for (u32 j = 0; j < (yblocks >> 1); j++)
			{
				top = (DXTColBlock*)(surface->getPixels() + j * linesize);
				bottom = (DXTColBlock*)(surface->getPixels() + (((yblocks - j) - 1) * linesize));

				(*flipblocks)(top, xblocks);
				(*flipblocks)(bottom, xblocks);

				swap(bottom, top, linesize);
			}
		}
	}

	void flip_texture(Face *texture, TextureFormat format)
	{
		flip(texture, format);

		for (u32 i = 0; i < texture->getMipmapCount(); i++)
		{
			flip(texture->getMipmap(i), format);
		}
	}

	bool TextureFactory::createDDSFromFile(const char* ddsFile, bool flipImage, bool srgb, Texture* texture, ArrayAllocator* textureAllocator, vx::StackAllocator* scratchAllocator)
	{
		vx::File inFile;
		if (!inFile.open(ddsFile, vx::FileAccess::Read))
		{
			return false;
		}

		auto fileSize = inFile.getSize();
		auto marker = scratchAllocator->getMarker();
		auto fileData = scratchAllocator->allocate(fileSize, 4);
		VX_ASSERT(fileData != nullptr);
		VX_ASSERT(fileSize == static_cast<u32>(fileSize));
		inFile.read(fileData, static_cast<u32>(fileSize));

		auto result = createDDSFromMemory(fileData, flipImage, srgb, texture, textureAllocator);

		scratchAllocator->clear(marker);

		return result;
	}

	bool TextureFactory::createDDSFromMemory(const u8* ddsData, bool flipImage, bool srgb, Texture* texture, ArrayAllocator* textureAllocator)
	{
		u32 magic;
		ddsData = vx::read(magic, ddsData);
		if (magic != 0x20534444)
		{
			return false;
		}

		DDS_HEADER header;
		ddsData = vx::read(header, ddsData);

		swap_endian(&header.dwSize);
		swap_endian(&header.dwFlags);
		swap_endian(&header.dwHeight);
		swap_endian(&header.dwWidth);
		swap_endian(&header.dwPitchOrLinearSize);
		swap_endian(&header.dwMipMapCount);
		swap_endian(&header.ddspf.dwSize);
		swap_endian(&header.ddspf.dwFlags);
		swap_endian(&header.ddspf.dwFourCC);
		swap_endian(&header.ddspf.dwRGBBitCount);
		swap_endian(&header.dwCaps);
		swap_endian(&header.dwCaps2);

		auto textureType = TextureType::Flat;
		u8 components = 4;
		auto textureFormat = TextureFormat::BGRA;
		u32 blockSize = 4;

		// check if image is a cubemap
		if (header.dwCaps2 & DDSCAPS2_CUBEMAP)
			textureType = TextureType::Cubemap;

		// check if image is a volume texture
		if ((header.dwCaps2 & DDSCAPS2_VOLUME) && (header.dwDepth > 0))
			textureType = TextureType::Volume;

		// figure out what the image format is
		if (header.ddspf.dwFlags & DDSF_FOURCC)
		{
			switch (header.ddspf.dwFourCC)
			{
			case FOURCC_DXT1:
				textureFormat = TextureFormat::DXT1;
				components = 3;
				blockSize = 8;
				break;
			case FOURCC_DXT3:
				textureFormat = TextureFormat::DXT3;
				components = 4;
				blockSize = 16;
				break;
			case FOURCC_DXT5:
				textureFormat = TextureFormat::DXT5;
				components = 4;
				blockSize = 16;
				break;
			case FOURCC_DX10:
			{
				DDS_HEADER_DXT10 dxtHeader;
				ddsData = vx::read(dxtHeader, ddsData);

				switch (dxtHeader.dxgiFormat)
				{
				case DXGI_FORMAT_BC7_UNORM_SRGB:
				{
					components = 4;
					blockSize = 16;
					textureFormat = TextureFormat::BC7_UNORM_SRGB;
				}break;
				case DXGI_FORMAT_BC7_UNORM:
				{
					components = 4;
					blockSize = 16;
					textureFormat = TextureFormat::BC7_UNORM;
				}break;

				case DXGI_FORMAT_BC6H_UF16:
				{
					components = 3;
					blockSize = 16;
					textureFormat = TextureFormat::BC6H_UF16;
				}break;

				case DXGI_FORMAT_BC6H_SF16:
				{
					components = 3;
					blockSize = 16;
					textureFormat = TextureFormat::BC6H_SF16;
				}break;
				default:
					return false;
					break;
				}
			}break;
			default:
				return false;
			}
		}
		else if (header.ddspf.dwFlags == DDSF_RGBA && header.ddspf.dwRGBBitCount == 32)
		{
			textureFormat = TextureFormat::BGRA;
			components = 4;
			blockSize = 4;
		}
		else if (header.ddspf.dwFlags == DDSF_RGB  && header.ddspf.dwRGBBitCount == 32)
		{
			textureFormat = TextureFormat::BGRA;
			components = 4;
			blockSize = 4;
		}
		else if (header.ddspf.dwFlags == DDSF_RGB  && header.ddspf.dwRGBBitCount == 24)
		{
			textureFormat = TextureFormat::BGR;
			components = 3;
			blockSize = 3;
		}
		else if (header.ddspf.dwRGBBitCount == 8)
		{
			textureFormat = TextureFormat::RED;
			components = 1;
			blockSize = 1;
		}
		else
		{
			return false;
		}

		vx::uint3 dimension;
		dimension.x = header.dwWidth;
		dimension.y = header.dwHeight;
		dimension.z = clamp_size(header.dwDepth);   // set to 1 if 0

													// use correct size calculation function depending on whether image is 
													// compressed
		u32(*sizefunc)(const vx::uint2 &dim, u32 blockSize);
		sizefunc = &detail::getSizeNormal;
		if (isCompressed(textureFormat))
		{
			sizefunc = &detail::getSizeBlock;
			/*if (isDxt(textureFormat))
			{
				sizefunc = &detail::getSizeBlock;
			else
			{
				sizefunc = &size_bc;
			}*/
		}

		auto faceCount = (u32)(textureType == TextureType::Cubemap ? 6 : 1);
		auto faces = textureAllocator->allocate<Face[]>(sizeof(Face) * faceCount, __alignof(Face));
		// load all surfaces for the image (6 surfaces for cubemaps)
		for (u32 n = 0; n < faceCount; ++n)
		{
			textureAllocator->construct<Face>(&faces[n]);
			// get reference to newly added texture object
			auto &face = faces[n];

			// calculate surface size
			u32 size = (*sizefunc)(vx::uint2(dimension.x, dimension.y), blockSize) * dimension.z;

			// load surface
			auto pixels = textureAllocator->allocate<u8[]>(size, 4);
			ddsData = vx::read(pixels.get(), ddsData, size);

			face.create(dimension, size, std::move(pixels));

			if (flipImage)
			{
				flip(&face, textureFormat);
			}

			vx::uint3 dim;
			dim.x = clamp_size(dimension.x >> 1);
			dim.y = clamp_size(dimension.y >> 1);
			dim.z = clamp_size(dimension.z >> 1);

			// store number of mipmaps
			u32 numMipmaps = header.dwMipMapCount;

			// number of mipmaps in file includes main surface so decrease count 
			// by one
			if (numMipmaps != 0)
				--numMipmaps;

			u32 w = dim.x;
			u32 h = dim.y;
			u32 mipmapCount = 0;
			for (u32 i = 0; i < numMipmaps && (w || h); i++)
			{
				++mipmapCount;

				w = clamp_size(w >> 1);
				h = clamp_size(h >> 1);
			}

			managed_ptr<Surface[]> mipmaps;
			if (mipmapCount != 0)
			{
				mipmaps = textureAllocator->allocate<Surface[]>(sizeof(Surface)*mipmapCount, __alignof(Surface));

				// load all mipmaps for current surface
				for (u32 i = 0; i < mipmapCount; i++)
				{
					textureAllocator->construct<Surface>(&mipmaps[i]);
					auto &mipmap = mipmaps[i];

					// calculate mipmap size
					auto mipmapSize = (*sizefunc)(vx::uint2(dim.x, dim.y), blockSize) * dim.z;

					auto mipampPixels = textureAllocator->allocate<u8[]>(size, 4);
					ddsData = vx::read(mipampPixels.get(), ddsData, mipmapSize);

					mipmap.create(dim, mipmapSize, std::move(mipampPixels));

					if (flipImage)
					{
						flip(&mipmap, textureFormat);
					}

					// shrink to next power of 2
					dim.x = clamp_size(dim.x >> 1);
					dim.y = clamp_size(dim.y >> 1);
					dim.z = clamp_size(dim.z >> 1);
				}
			}
			face.setMipmaps(std::move(mipmaps), mipmapCount);
		}

		// swap cubemaps on y axis (since image is flipped in OGL)
		if (textureType == TextureType::Cubemap && flipImage)
		{
			Face tmp;
			tmp = std::move(faces[3]);
			faces[3] = std::move(faces[2]);
			faces[2] = std::move(tmp);
		}
		texture->create(std::move(faces), textureFormat, textureType, components);
		return true;
	}

	void flipImage(u8 *data, u32 width, u32 height, u8 n)
	{
		u32 width_in_bytes = width * n;
		u8 temp = 0;
		u32 half_height = height / 2;

		for (u32 row = 0; row < half_height; row++)
		{
			auto top = data + row * width_in_bytes;
			auto bottom = data + (height - row - 1) * width_in_bytes;
			for (u32 col = 0; col < width_in_bytes; col++)
			{
				temp = *top;
				*top = *bottom;
				*bottom = temp;
				top++;
				bottom++;
			}
		}
	}

	bool TextureFactory::createPngFromFile(const char* pngFile, bool flip, bool srgb, Texture* texture, ArrayAllocator* textureAllocator, vx::StackAllocator* scratchAllocator)
	{
		vx::File inFile;
		if (!inFile.open(pngFile, vx::FileAccess::Read))
		{
			return false;
		}

		auto fileSize = inFile.getSize();

		auto marker = scratchAllocator->getMarker();
		auto fileData = scratchAllocator->allocate(fileSize);
		VX_ASSERT(fileData != nullptr);
		VX_ASSERT(fileSize == static_cast<u32>(fileSize));

		inFile.read(fileData, static_cast<u32>(fileSize));

		auto result = createPngFromMemory(fileData, static_cast<u32>(fileSize), flip, srgb, texture, textureAllocator);

		scratchAllocator->clear(marker);

		return result;
	}

	bool TextureFactory::createPngFromMemory(const u8* pngData, u32 size, bool flip, bool srgb, Texture* texture, ArrayAllocator* textureAllocator)
	{
		s32 x, y, comp;
		auto data = stbi_load_from_memory(pngData, size, &x, &y, &comp, 0);
		if (data == nullptr)
			return false;

		if (flip)
		{
			flipImage(data, x, y, comp);
		}

		if (comp == 3)
		{
			auto oldDataSize = x * y * comp;
			auto newDataSize = x * y * 4;

			auto newData = std::make_unique<u8[]>(newDataSize);

			auto newPtr = newData.get();
			auto oldPtr = data;
			for (u32 yy = 0; yy < static_cast<u32>(y); ++yy)
			{
				for (u32 xx = 0; xx < static_cast<u32>(x); ++xx)
				{
					newPtr[0] = oldPtr[0];
					newPtr[1] = oldPtr[1];
					newPtr[2] = oldPtr[2];
					newPtr[3] = 255;

					oldPtr += 3;
					newPtr += 4;
				}
			}

			comp = 4;

			stbi_image_free(data);
			data = newData.release();
		}


		auto dataSize = x * y * comp;

		auto face = textureAllocator->allocate<Face[]>(sizeof(Face), __alignof(Face));
		auto faceData = textureAllocator->allocate<u8[]>(dataSize, 4);
		if (faceData.get() == nullptr)
		{
			VX_ASSERT(false);
		}


		memcpy(faceData.get(), data, dataSize);

		stbi_image_free(data);

		textureAllocator->construct<Face>(&face[0]);
		face[0].create(vx::uint3(x, y, 1), dataSize, std::move(faceData));


		TextureFormat format = TextureFormat::RGBA;
		if (srgb)
		{
			switch (comp)
			{
			case 4:
				format = TextureFormat::SRGBA;
				break;
			default:
				VX_ASSERT(false);
				break;
			}
		}
		else
		{
			switch (comp)
			{
			case 4:
				format = TextureFormat::RGBA;
				break;
			case 3:
				format = TextureFormat::RGB;
				break;
			case 2:
				format = TextureFormat::RG;
				break;
			case 1:
				format = TextureFormat::RED;
				break;
			default:
				break;
			}
		}

		texture->create(std::move(face), format, TextureType::Flat, comp);
		return true;
	}
}