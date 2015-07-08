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

#include <vxLib/types.h>
#include <vector>
#include <vxEngineLib/Graphics/Texture.h>

struct DXTColBlock;

struct DXT3AlphaBlock;

struct DXT5AlphaBlock;

class DDS_File
{
	enum class Type{ Flat, Cubemap, Volume };

	std::vector<Graphics::Face> m_images;
	u32 m_components;
	Type m_type;
	u32 m_format;

	u32 clamp_size(u32 size);

	u32 size_dxtc(unsigned int width, unsigned int height);
	u32 size_rgb(unsigned int width, unsigned int height);
	u32 size_bc(u32 width, u32 height);

	inline void swap_endian(void *val);

	void flip(Graphics::Surface* surface);

	void flip_texture(Graphics::Face* texture);

	void swap(void *byte1, void *byte2, unsigned int size);

	void flip_blocks_dxtc1(DXTColBlock *line, unsigned int numBlocks);

	void flip_blocks_dxtc3(DXTColBlock *line, unsigned int numBlocks);

	void flip_dxt5_alpha(DXT5AlphaBlock *block);

	void flip_blocks_dxtc5(DXTColBlock *line, unsigned int numBlocks);

public:
	DDS_File();
	~DDS_File();

	bool loadFromFile(const char* file, bool flipImage = true);

	bool isCompressed() const;
	bool isDxt() const;

	const Graphics::Face& getTexture(u32 i) const { return m_images[i]; }

	void clear();
};
