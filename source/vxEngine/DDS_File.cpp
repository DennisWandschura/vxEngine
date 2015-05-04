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
#include "DDS_File.h"
#include <cstdio>
#include <vxLib/gl/gl.h>
#include <assert.h>

struct DXTColBlock
{
	unsigned short col0;
	unsigned short col1;

	unsigned char row[4];
};

struct DXT3AlphaBlock
{
	unsigned short row[4];
};

struct DXT5AlphaBlock
{
	unsigned char alpha0;
	unsigned char alpha1;

	unsigned char row[6];
};

Surface::Surface()
	: m_width(0),
	m_height(0),
	m_depth(0),
	m_size(0),
	m_pixels(nullptr)
{
}

Surface::Surface(Surface &&rhs)
	:m_width(rhs.m_width),
	m_height(rhs.m_height),
	m_depth(rhs.m_depth),
	m_size(rhs.m_size),
	m_pixels(rhs.m_pixels)
{
	rhs.m_pixels = nullptr;
}

Surface::~Surface()
{
	clear();
}

Surface& Surface::operator = (Surface &&rhs)
{
	if (this != &rhs)
	{
		std::swap(m_width, rhs.m_width);
		std::swap(m_height, rhs.m_height);
		std::swap(m_depth, rhs.m_depth);
		std::swap(m_size, rhs.m_size);
		std::swap(m_pixels, rhs.m_pixels);
	}
	return *this;
}

void Surface::create(U32 width, U32 height, U32 depth, U32 imgsize, U8* pixels)
{
	assert(width != 0);
	assert(height != 0);
	assert(depth != 0);
	assert(imgsize != 0);
	assert(pixels);

	clear();

	m_width = width;
	m_height = height;
	m_depth = depth;
	m_size = imgsize;
	m_pixels = new U8[imgsize];
	memcpy(m_pixels, pixels, imgsize);
}

void Surface::clear()
{
	if (m_pixels != nullptr)
	{
		delete[] m_pixels;
		m_pixels = nullptr;
	}
}

Texture::Texture()
	:Surface(),
	m_mipmaps()
{
}

Texture::Texture(Texture &&rhs)
	: Surface(std::move(rhs)),
	m_mipmaps(std::move(rhs.m_mipmaps))
{
}

Texture& Texture::operator = (Texture &&rhs)
{
	Surface::operator=(std::move(rhs));
	if (this != &rhs)
	{
		std::swap(m_mipmaps, rhs.m_mipmaps);
	}
	return *this;
}

void Texture::create(U32 width, U32 height, U32 depth, U32 size, U8* pixels)
{
	Surface::create(width, height, depth, size, pixels);

	m_mipmaps.clear();
}

void Texture::clear()
{
	Surface::clear();

	m_mipmaps.clear();
}

struct DDS_PIXELFORMAT {
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
};

typedef struct {
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
} DDS_HEADER;

#define DDS_FOURCC      0x00000004  // DDPF_FOURCC
#define DDS_RGB         0x00000040  // DDPF_RGB
#define DDS_RGBA        0x00000041  // DDPF_RGB | DDPF_ALPHAPIXELS
#define DDS_LUMINANCE   0x00020000  // DDPF_LUMINANCE
#define DDS_LUMINANCEA  0x00020001  // DDPF_LUMINANCE | DDPF_ALPHAPIXELS
#define DDS_ALPHA       0x00000002  // DDPF_ALPHA
#define DDS_PAL8        0x00000020  // DDPF_PALETTEINDEXED8

// pixel format flags
const unsigned long DDSF_ALPHAPIXELS = 0x00000001l;
const unsigned long DDSF_FOURCC = 0x00000004l;
const unsigned long DDSF_RGB = 0x00000040l;
const unsigned long DDSF_RGBA = 0x00000041l;

const unsigned long FOURCC_DXT1 = 0x31545844l; //(MAKEFOURCC('D','X','T','1'))
const unsigned long FOURCC_DXT3 = 0x33545844l; //(MAKEFOURCC('D','X','T','3'))
const unsigned long FOURCC_DXT5 = 0x35545844l; //(MAKEFOURCC('D','X','T','5'))

#define DDSCAPS2_CUBEMAP 0x200
#define DDSCAPS2_VOLUME 0x200000

unsigned int DDS_File::clamp_size(unsigned int size)
{
	if (size <= 0)
		size = 1;

	return size;
}

///////////////////////////////////////////////////////////////////////////////
// calculates size of DXTC texture in bytes
unsigned int DDS_File::size_dxtc(unsigned int width, unsigned int height)
{
	return ((width + 3) / 4)*((height + 3) / 4)*
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
}

///////////////////////////////////////////////////////////////////////////////
// calculates size of uncompressed RGB texture in bytes
unsigned int DDS_File::size_rgb(unsigned int width, unsigned int height)
{
	return width*height*m_components;
}

///////////////////////////////////////////////////////////////////////////////
// Swap the bytes in a 32 bit value
void DDS_File::swap_endian(void *val)
{
#ifdef MACOS
	unsigned int *ival = (unsigned int *)val;

	*ival = ((*ival >> 24) & 0x000000ff) |
		((*ival >> 8) & 0x0000ff00) |
		((*ival << 8) & 0x00ff0000) |
		((*ival << 24) & 0xff000000);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// flip image around X axis
void DDS_File::flip(Surface* surface)
{
	unsigned int linesize;
	unsigned int offset;

	if (!isCompressed())
	{
		assert(surface->getDepth() > 0);

		unsigned int imagesize = surface->getSize() / surface->getDepth();
		linesize = imagesize / surface->getHeight();

		for (unsigned int n = 0; n < surface->getDepth(); n++)
		{
			offset = imagesize*n;
			unsigned char *top = (unsigned char*)surface + offset;
			unsigned char *bottom = top + (imagesize - linesize);

			for (unsigned int i = 0; i < (surface->getHeight() >> 1); i++)
			{
				swap(bottom, top, linesize);

				top += linesize;
				bottom -= linesize;
			}
		}
	}
	else
	{
		void (DDS_File::*flipblocks)(DXTColBlock*, unsigned int);
		U32 xblocks = surface->getWidth() / 4;
		U32 yblocks = surface->getHeight() / 4;
		U32 blocksize;

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

		linesize = xblocks * blocksize;

		DXTColBlock *top;
		DXTColBlock *bottom;

		for (unsigned int j = 0; j < (yblocks >> 1); j++)
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

	for (unsigned int i = 0; i < texture->getMipmapCount(); i++)
	{
		flip(texture->getMipmap(i));
	}
}

///////////////////////////////////////////////////////////////////////////////
// swap to sections of memory
void DDS_File::swap(void *byte1, void *byte2, unsigned int size)
{
	unsigned char *tmp = new unsigned char[size];

	memcpy(tmp, byte1, size);
	memcpy(byte1, byte2, size);
	memcpy(byte2, tmp, size);

	delete[] tmp;
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT1 color block
void DDS_File::flip_blocks_dxtc1(DXTColBlock *line, unsigned int numBlocks)
{
	DXTColBlock *curblock = line;

	for (unsigned int i = 0; i < numBlocks; i++)
	{
		swap(&curblock->row[0], &curblock->row[3], sizeof(unsigned char));
		swap(&curblock->row[1], &curblock->row[2], sizeof(unsigned char));

		curblock++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT3 color block
void DDS_File::flip_blocks_dxtc3(DXTColBlock *line, unsigned int numBlocks)
{
	DXTColBlock *curblock = line;
	DXT3AlphaBlock *alphablock;

	for (unsigned int i = 0; i < numBlocks; i++)
	{
		alphablock = (DXT3AlphaBlock*)curblock;

		swap(&alphablock->row[0], &alphablock->row[3], sizeof(unsigned short));
		swap(&alphablock->row[1], &alphablock->row[2], sizeof(unsigned short));

		curblock++;

		swap(&curblock->row[0], &curblock->row[3], sizeof(unsigned char));
		swap(&curblock->row[1], &curblock->row[2], sizeof(unsigned char));

		curblock++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// flip a DXT5 alpha block
void DDS_File::flip_dxt5_alpha(DXT5AlphaBlock *block)
{
	unsigned char gBits[4][4];

	const unsigned long mask = 0x00000007;          // bits = 00 00 01 11
	unsigned long bits = 0;
	memcpy(&bits, &block->row[0], sizeof(unsigned char) * 3);

	gBits[0][0] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[0][1] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[0][2] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[0][3] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[1][0] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[1][1] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[1][2] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[1][3] = (unsigned char)(bits & mask);

	bits = 0;
	memcpy(&bits, &block->row[3], sizeof(unsigned char) * 3);

	gBits[2][0] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[2][1] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[2][2] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[2][3] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[3][0] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[3][1] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[3][2] = (unsigned char)(bits & mask);
	bits >>= 3;
	gBits[3][3] = (unsigned char)(bits & mask);

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
void DDS_File::flip_blocks_dxtc5(DXTColBlock *line, unsigned int numBlocks)
{
	DXTColBlock *curblock = line;
	DXT5AlphaBlock *alphablock;

	for (unsigned int i = 0; i < numBlocks; i++)
	{
		alphablock = (DXT5AlphaBlock*)curblock;

		flip_dxt5_alpha(alphablock);

		curblock++;

		swap(&curblock->row[0], &curblock->row[3], sizeof(unsigned char));
		swap(&curblock->row[1], &curblock->row[2], sizeof(unsigned char));

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
	unsigned int width, height, depth;
	width = header.dwWidth;
	height = header.dwHeight;
	depth = clamp_size(header.dwDepth);   // set to 1 if 0

	// use correct size calculation function depending on whether image is 
	// compressed
	unsigned int (DDS_File::*sizefunc)(unsigned int, unsigned int);
	sizefunc = (isCompressed() ? &DDS_File::size_dxtc : &DDS_File::size_rgb);

	// load all surfaces for the image (6 surfaces for cubemaps)
	for (unsigned int n = 0; n < (unsigned int)(m_type == Type::Cubemap ? 6 : 1); ++n)
	{
		// add empty texture object
		m_images.push_back(Texture());

		// get reference to newly added texture object
		auto &img = m_images[n];

		// calculate surface size
		unsigned int size = (this->*sizefunc)(width, height)*depth;

		// load surface
		unsigned char *pixels = new unsigned char[size];
		fread(pixels, 1, size, inFile);

		img.create(width, height, depth, size, pixels);

		delete[] pixels;

		if (flipImage)
			flip(&img);

		unsigned int w = clamp_size(width >> 1);
		unsigned int h = clamp_size(height >> 1);
		unsigned int d = clamp_size(depth >> 1);

		// store number of mipmaps
		unsigned int numMipmaps = header.dwMipMapCount;

		// number of mipmaps in file includes main surface so decrease count 
		// by one
		if (numMipmaps != 0)
			numMipmaps--;

		// load all mipmaps for current surface
		for (unsigned int i = 0; i < numMipmaps && (w || h); i++)
		{
			// add empty surface
			img.addMipmap(Surface());

			// get reference to newly added mipmap
			Surface* mipmap = img.getMipmap(i);

			// calculate mipmap size
			size = (this->*sizefunc)(w, h)*d;

			unsigned char *pixels = new unsigned char[size];
			fread(pixels, 1, size, inFile);

			mipmap->create(w, h, d, size, pixels);

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
		(m_format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT))
		return true;
	else
		return false;
}

void DDS_File::clear()
{
	m_images.clear();
	m_components = 0;
	m_type = Type::Flat;
	m_format = 0;
}