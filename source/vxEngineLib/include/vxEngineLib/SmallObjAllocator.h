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
#pragma once

#include <vxLib/types.h>
#include <vector>
#include <mutex>

struct Chunk
{
	u8* ptr{ nullptr };
	u16 freeBlocks{ 0 };
	u16 firstFreeBlock{ 0 };

	Chunk() = default;

	void init(u32 blockSize, u16 blockCount);

	u8* allocate(u32 blockSize);

	void deallocate(u8* p, u32 blockSize);

	u8 contains(u8* p, u32 blockSize, u8 blockCount);
};

class ChunkAllocator
{
	Chunk* m_pChunks{ nullptr };
	u16 m_blockSize{ 0 };
	u16 m_blockCount{ 0 };
	u16 m_size{ 0 };
	u16 m_capacity{ 0 };
	u16 m_allocChunk{ 0 };
	u16 m_deallocChunk{ 0 };

	// adds a new chuck and returns its index
	u16 push_back();

public:
	ChunkAllocator() = default;
	ChunkAllocator(const ChunkAllocator&) = delete;
	ChunkAllocator(ChunkAllocator &&rhs);

	ChunkAllocator& operator=(const ChunkAllocator&) = delete;
	ChunkAllocator& operator=(ChunkAllocator &&rhs);

	~ChunkAllocator();

	void init(u16 blockSize, u16 blockCount);

	u8* allocate();

	// make sure p is not nullptr, and call contains() before calling deallocate
	void deallocate(u8* p);

	u8 contains(u8* p);

	u16 getBlockSize() const { return m_blockSize; }
};

class SmallObjAllocator
{
	static const u16 s_maxObjSize = 0xffffu;

	std::mutex m_mutex;
	std::vector<ChunkAllocator> m_allocators;
	u32 m_lastAlloc;
	u32 m_lastDealloc;
	u32 m_chunkSize;

	u32 createAllocator(u16 size);
	void sortAllocators();

public:
	explicit SmallObjAllocator(u32 chunkSize)
		:m_allocators(), m_lastAlloc(0), m_lastDealloc(0), m_chunkSize(chunkSize) {}

	u8* allocate(u32 size);

	bool deallocate(u8* p, u32 size);
};