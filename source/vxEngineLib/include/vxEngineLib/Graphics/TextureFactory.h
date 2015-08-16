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

class ArrayAllocator;

namespace vx
{
	class StackAllocator;
}

#include <vxLib/math/Vector.h>

namespace Graphics
{
	class Texture;
	enum TextureFormat : u8;

	class TextureFactory
	{
	public:
		static bool createDDSFromFile(const char* ddsFile, bool flipImage, bool srgb, Texture* texture, ArrayAllocator* textureAllocator, vx::StackAllocator* scratchAllocator);
		static bool createPngFromFile(const char* pngFile, bool flipImage, bool srgb, Texture* texture, ArrayAllocator* textureAllocator, vx::StackAllocator* scratchAllocator);

		static bool createDDSFromMemory(const u8* ddsData, bool flipImage, bool srgb, Texture* texture, ArrayAllocator* textureAllocator);
		static bool createPngFromMemory(const u8* pngData, u32 size, bool flipImage, bool srgb, Texture* texture, ArrayAllocator* textureAllocator);

		static u32 getTextureSize(TextureFormat format, const vx::uint2 &dim);
		static u32 getRowPitch(TextureFormat format, u32 width);
	};
}
