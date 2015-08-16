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

struct ID3D12Resource;

namespace vx
{
	class StackAllocator;
}

namespace d3d
{
	class Heap;
	class Device;
}

namespace Graphics
{
	class Texture;
}

#include <vxLib/math/Vector.h>
#include "Freelist.h"
#include "d3d.h"
#include <vxLib/StringID.h>

class TextureManager
{
	struct Entry;

	Freelist m_freelist;
	Entry* m_entries;
	u32 m_capacity;
	u32 m_format;
	d3d::Resource m_textureBuffer;

	bool createTextureBuffer(const vx::uint3 &textureDim, u32 dxgiFormat, u32* heapOffset, d3d::Heap* heap, d3d::Device* device);

public:
	TextureManager();
	~TextureManager();

	bool initialize(vx::StackAllocator* allocator, const vx::uint3 &textureDim, u32 dxgiFormat, u32* heapOffset, d3d::Heap* heap, d3d::Device* device);

	bool addTexture(const vx::StringID &sid, const Graphics::Texture &texture, u32* slice);
	bool removeTexture(const vx::StringID &sid);
};