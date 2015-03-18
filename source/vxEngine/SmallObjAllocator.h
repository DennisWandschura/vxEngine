#pragma once

#include <vxLib/types.h>
#include <vector>

struct Chunk
{
	U8* ptr{ nullptr };
	U16 freeBlocks{ 0 };
	U16 firstFreeBlock{ 0 };

	Chunk() = default;

	void init(U32 blockSize, U16 blockCount);

	U8* allocate(U32 blockSize);

	void deallocate(U8* p, U32 blockSize);

	U8 contains(U8* p, U32 blockSize, U8 blockCount);
};

class ChunkAllocator
{
	Chunk* m_pChunks{ nullptr };
	U16 m_blockSize{ 0 };
	U16 m_blockCount{ 0 };
	U8 m_size{ 0 };
	U8 m_capacity{ 0 };
	U8 m_allocChunk{ 0 };
	U8 m_deallocChunk{ 0 };

	// adds a new chuck and returns its index
	U16 push_back()
	{
		assert(m_size != 0xff);
		if (m_size == m_capacity)
		{
			auto newCap = m_capacity + 1;

			auto ptr = new Chunk[newCap];

			std::move(m_pChunks, m_pChunks + m_size, ptr);

			std::swap(ptr, m_pChunks);
			m_capacity = newCap;

			delete[](ptr);
		}

		Chunk newChunk;
		newChunk.init(m_blockSize, m_blockCount);

		m_pChunks[m_size] = newChunk;

		auto index = m_size;
		++m_size;

		return index;
	}

public:
	ChunkAllocator() = default;
	ChunkAllocator(const ChunkAllocator&) = delete;
	ChunkAllocator(ChunkAllocator &&rhs);

	ChunkAllocator& operator=(const ChunkAllocator&) = delete;
	ChunkAllocator& operator=(ChunkAllocator &&rhs);

	~ChunkAllocator();

	void init(U16 blockSize, U16 blockCount);

	U8* allocate();

	// make sure p is not nullptr, and call contains() before calling deallocate
	void deallocate(U8* p);

	U8 contains(U8* p);

	U16 getBlockSize() const { return m_blockSize; }
};

class SmallObjAllocator
{
	static const U16 s_maxObjSize = 0xffffu;

	std::vector<ChunkAllocator> m_allocators;
	U32 m_lastAlloc{ 0 };
	U32 m_lastDealloc{ 0 };
	U32 m_chunkSize{ 0 };

	U32 createAllocator(U16 size);
	void sortAllocators();

public:
	explicit SmallObjAllocator(U32 chunkSize)
		:m_chunkSize(chunkSize)
	{
	}

	U8* allocate(U32 size);

	void deallocate(U8* p, U32 size);
};