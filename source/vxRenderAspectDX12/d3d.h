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

#include "d3dObject.h"

struct ID3D12Resource;
struct ID3D12Device;

namespace d3d
{
	inline s32 getAlignedSize(s32 size, s32 alignment)
	{
		alignment = alignment - 1;
		return (size + alignment) & ~alignment;
	}

	inline u32 getAlignedSize(u32 size, u32 alignment)
	{
		alignment = alignment - 1;
		return (size + alignment) & ~alignment;
	}

	inline u64 getAlignedSize(u64 size, u64 alignment)
	{
		alignment = alignment - 1;
		return (size + alignment) & ~alignment;
	}

	template<size_t SZ, size_t ALIGNMENT>
	struct AlignedSize
	{
		enum : size_t { size = SZ + (ALIGNMENT - 1) & ~(ALIGNMENT - 1) };
	};

	template<typename T, size_t COUNT, size_t ALIGNMENT>
	struct AlignedSizeType
	{
		enum : size_t { size = (sizeof(T) * COUNT) + (ALIGNMENT - 1) & ~(ALIGNMENT - 1) };
	};
}