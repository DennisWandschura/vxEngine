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
#include <vxRenderAspect/DDS_File.h>
#include <vxRenderAspect/dds.h>
#include <cstdio>
#include <vxGL/gl.h>
#include <assert.h>

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



Texture::Texture()
	:Surface(),
	m_mipmaps()
{
}

Texture::Texture(Texture &&rhs)
	: Graphics::Surface(std::move(rhs)),
	m_mipmaps(std::move(rhs.m_mipmaps))
{
}

Texture& Texture::operator = (Texture &&rhs)
{
	Graphics::Surface::operator=(std::move(rhs));
	if (this != &rhs)
	{
		std::swap(m_mipmaps, rhs.m_mipmaps);
	}
	return *this;
}

void Texture::create(const vx::uint3 &dimension, u32 size, u8* pixels)
{
	Graphics::Surface::create(dimension, size, pixels);

	m_mipmaps.clear();
}

void Texture::clear()
{
	Graphics::Surface::clear();

	m_mipmaps.clear();
}

DDS_File::DDS_File()
	:m_images(),
	m_components(0),
	m_type(),
	m_format(0)
{
}

DDS_File::~DDS_File()
{
}

u32 DDS_File::clamp_size(u32 size)
{
	if (size <= 0)
		size = 1;

	return size;
}

///////////////////////////////////////////////////////////////////////////////
// calculates size of DXTC texture in bytes
u32 DDS_File::size_dxtc(u32 width, u32 height)
{
	return ((width + 3) / 4)*((height + 3) / 4)*
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
}

///////////////////////////////////////////////////////////////////////////////
// calculates size of uncompressed RGB texture in bytes
u32 DDS_File::size_rgb(u32 width, u32 height)
{
	return width*height*m_components;
}

u32 DDS_File::size_bc(u32 width, u32 height)
{
	return ((width + 3) / 4)*((height + 3) / 4)*16;
}

///////////////////////////////////////////////////////////////////////////////
// Swap the bytes in a 32 bit value
void DDS_File::swap_endian(void *val)
{
#ifdef MACOS
	u32 *ival = (u32 *)val;

	*ival = ((*ival >> 24) & 0x000000ff) |
		((*ival >> 8) & 0x0000ff00) |
		((*ival << 8) & 0x00ff0000) |
		((*ival << 24) & 0xff000000);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// flip image around X axis
void DDS_File::flip(Graphics::Surface* surface)
{
	auto dim = surface->getDimension();

	if (!isCompressed())
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
		void (DDS_File::*flipblocks)(DXTColBlock*, u32);
		u32 xblocks = dim.x / 4;
		u32 yblocks = dim.y / 4;
		u32 blocksize;

		switch (m_format)
		{
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			blocksize = 8;
			flipblocks = &DDS_File::flip_blocks_dxtc1;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			blocksize = 16;
			flipblocks = &DDS_File::flip_blocks_dxtc3;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			blocksize = 16;
			flipblocks = &DDS_File::flip_blocks_dxtc5;
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

			(this->*flipblocks)(top, xblocks);
			(this->*flipblocks)(bottom, xblocks);

			swap(bottom, top, linesize);
		}
	}
}

void DDS_File::flip_texture(Texture *texture)
{
	flip(texture);

	for (u32 i = 0; i < texture->getMipmapCount(); i++)
	{
		flip(texture->getMipmap(i));
	}
}

///////////////////////////////////////////////////////////////////////////////
// swap to sections of memory
void DDS_File::swap(void *byte1, void *byte2, u32 size)
{
	u8 *tmp = new u8[size];

	memcpy(tmp, byte1, size);
	memcpy(byte1, byte2, size);
	memcpy(byte2, tmp, size);

	delete[] tmp;
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT1 color block
void DDS_File::flip_blocks_dxtc1(DXTColBlock *line, u32 numBlocks)
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
void DDS_File::flip_blocks_dxtc3(DXTColBlock *line, u32 numBlocks)
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
void DDS_File::flip_dxt5_alpha(DXT5AlphaBlock *block)
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
void DDS_File::flip_blocks_dxtc5(DXTColBlock *line, u32 numBlocks)
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

bool DDS_File::loadFromFile(const char* file, bool flipImage)
{
	FILE* inFile = fopen(file, "rb");
	if (!inFile)
	{
		return false;
	}

	DWORD magic;
	fread(&magic, 4, 1, inFile);
	if (magic != 0x20534444)
	{
		fclose(inFile);
		return false;
	}

	DDS_HEADER header;
	fread(&header, sizeof(DDS_HEADER), 1, inFile);

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

	m_type = Type::Flat;

	// check if image is a cubemap
	if (header.dwCaps2 & DDSCAPS2_CUBEMAP)
		m_type = Type::Cubemap;

	// check if image is a volume texture
	if ((header.dwCaps2 & DDSCAPS2_VOLUME) && (header.dwDepth > 0))
		m_type = Type::Volume;

	// figure out what the image format is
	if (header.ddspf.dwFlags & DDSF_FOURCC)
	{
		switch (header.ddspf.dwFourCC)
		{
		case FOURCC_DXT1:
			m_format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			m_components = 3;
			break;
		case FOURCC_DXT3:
			m_format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			m_components = 4;
			break;
		case FOURCC_DXT5:
			m_format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			m_components = 4;
			break;
		case FOURCC_DX10:
		{
			DDS_HEADER_DXT10 dxtHeader;
			fread(&dxtHeader, sizeof(DDS_HEADER_DXT10), 1, inFile);

			switch (dxtHeader.dxgiFormat)
			{
			case DXGI_FORMAT_BC7_UNORM_SRGB:
			{
				m_components = 4;
				m_format = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM;
			}break;
			case DXGI_FORMAT_BC7_UNORM:
			{
				m_components = 4;
				m_format = GL_COMPRESSED_RGBA_BPTC_UNORM;
			}break;

			case DXGI_FORMAT_BC6H_UF16:
			{
				m_components = 3;
				m_format = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT;
			}break;

			case DXGI_FORMAT_BC6H_SF16:
			{
				m_components = 3;
				m_format = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT;
			}break;
			default:
				fclose(inFile);
				return false;
				break;
			}
		}break;
		default:
			fclose(inFile);
			return false;
		}
	}
	else if (header.ddspf.dwFlags == DDSF_RGBA && header.ddspf.dwRGBBitCount == 32)
	{
		m_format = GL_BGRA;
		m_components = 4;
	}
	else if (header.ddspf.dwFlags == DDSF_RGB  && header.ddspf.dwRGBBitCount == 32)
	{
		m_format = GL_BGRA;
		m_components = 4;
	}
	else if (header.ddspf.dwFlags == DDSF_RGB  && header.ddspf.dwRGBBitCount == 24)
	{
		m_format = GL_BGR;
		m_components = 3;
	}
	else if (header.ddspf.dwRGBBitCount == 8)
	{
		m_format = GL_RED;
		m_components = 1;
	}
	else
	{
		fclose(inFile);
		return false;
	}

	// store primary surface width/height/depth
	vx::uint3 dimension;
	dimension.x = header.dwWidth;
	dimension.y = header.dwHeight;
	dimension.z = clamp_size(header.dwDepth);   // set to 1 if 0

	// use correct size calculation function depending on whether image is 
	// compressed
	u32 (DDS_File::*sizefunc)(u32, u32);
	sizefunc = &DDS_File::size_rgb;
	if (isCompressed())
	{
		if (isDxt())
		{
			sizefunc = &DDS_File::size_dxtc;
		}
		else
		{
			sizefunc = &DDS_File::size_bc;
		}
	}

	// load all surfaces for the image (6 surfaces for cubemaps)
	for (u32 n = 0; n < (u32)(m_type == Type::Cubemap ? 6 : 1); ++n)
	{
		// add empty texture object
		m_images.push_back(Texture());

		// get reference to newly added texture object
		auto &img = m_images[n];

		// calculate surface size
		u32  size = (this->*sizefunc)(dimension.x, dimension.y)*dimension.z;

		// load surface
		u8 *pixels = new u8[size];
		fread(pixels, 1, size, inFile);

		img.create(dimension, size, pixels);

		delete[] pixels;

		if (flipImage)
			flip(&img);

		u32 w = clamp_size(dimension.x >> 1);
		u32 h = clamp_size(dimension.y >> 1);
		u32 d = clamp_size(dimension.z >> 1);

		// store number of mipmaps
		u32 numMipmaps = header.dwMipMapCount;

		// number of mipmaps in file includes main surface so decrease count 
		// by one
		if (numMipmaps != 0)
			numMipmaps--;

		// load all mipmaps for current surface
		for (u32 i = 0; i < numMipmaps && (w || h); i++)
		{
			// add empty surface
			img.addMipmap(Graphics::Surface());

			// get reference to newly added mipmap
			Graphics::Surface* mipmap = img.getMipmap(i);

			// calculate mipmap size
			size = (this->*sizefunc)(w, h)*d;

			u8 *pixels = new u8[size];
			fread(pixels, 1, size, inFile);

			mipmap->create(dimension, size, pixels);

			delete[] pixels;

			if (flipImage) flip(mipmap);

			// shrink to next power of 2
			w = clamp_size(w >> 1);
			h = clamp_size(h >> 1);
			d = clamp_size(d >> 1);
		}
	}

	// swap cubemaps on y axis (since image is flipped in OGL)
	if(m_type == Type::Cubemap && flipImage)
	{
		Texture tmp;
		tmp = std::move(m_images[3]);
		m_images[3] = std::move(m_images[2]);
		m_images[2] = std::move(tmp);
	}

	fclose(inFile);
	return true;
}

inline bool DDS_File::isCompressed() const
{
	if ((m_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) ||
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) ||
		(m_format == GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM) ||
		(m_format == GL_COMPRESSED_RGBA_BPTC_UNORM) ||
		(m_format == GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT) ||
		(m_format == GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT))
		return true;
	else
		return false;
}

bool DDS_File::isDxt() const
{
	return ((m_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ||
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT) ||
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT));
}

void DDS_File::clear()
{
	m_images.clear();
	m_components = 0;
	m_type = Type::Flat;
	m_format = 0;
}