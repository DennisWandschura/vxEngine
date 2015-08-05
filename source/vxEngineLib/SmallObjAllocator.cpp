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
#include <vxEngineLib/SmallObjAllocator.h>
#include <algorithm>

void Chunk::init(u32 blockSize, u16 blockCount)
{
	ptr = new u8[blockSize * blockCount];
	freeBlocks = blockCount;
	firstFreeBlock = 0;

	auto p = ptr;
	for (u32 i = 0; i < blockCount; p += blockSize)
	{
		*p = ++i;
	}
}

u8* Chunk::allocate(u32 blockSize)
{
	if (freeBlocks == 0)
		return nullptr;

	u8* p = ptr + (firstFreeBlock * blockSize);

	firstFreeBlock = *p;
	--freeBlocks;

	return p;
}

void Chunk::deallocate(u8* p, u32 blockSize)
{
	VX_ASSERT(p >= ptr);
	auto toRelease = p;
	VX_ASSERT((toRelease - ptr) % blockSize == 0);
	*toRelease = firstFreeBlock;
	firstFreeBlock = ((toRelease - ptr) / blockSize);
	VX_ASSERT(firstFreeBlock == (toRelease - ptr) / blockSize);

	++freeBlocks;
}

u8 Chunk::contains(u8* p, u32 blockSize, u8 blockCount)
{
	auto cmp = (p >= ptr) & (p < (ptr + blockSize * blockCount));
	return cmp;
}

ChunkAllocator::ChunkAllocator(ChunkAllocator &&rhs)
	:m_pChunks(rhs.m_pChunks),
	m_blockSize(rhs.m_blockSize),
	m_blockCount(rhs.m_blockCount),
	m_size(rhs.m_size),
	m_capacity(rhs.m_capacity),
	m_allocChunk(rhs.m_allocChunk),
	m_deallocChunk(rhs.m_deallocChunk)
{
	rhs.m_pChunks = nullptr;
}

ChunkAllocator& ChunkAllocator::operator=(ChunkAllocator &&rhs)
{
	if (this != &rhs)
	{
		std::swap(m_pChunks, rhs.m_pChunks);
		std::swap(m_blockSize, rhs.m_blockSize);
		std::swap(m_blockCount, rhs.m_blockCount);
		std::swap(m_size, rhs.m_size);
		std::swap(m_capacity, rhs.m_capacity);
		std::swap(m_allocChunk, rhs.m_allocChunk);
		std::swap(m_deallocChunk, rhs.m_deallocChunk);
	}
	return *this;
}

ChunkAllocator::~ChunkAllocator()
{
	delete[](m_pChunks);
	m_pChunks = nullptr;
}

void ChunkAllocator::init(u16 blockSize, u16 blockCount)
{
	m_blockSize = blockSize;
	m_blockCount = blockCount;
}

u16 ChunkAllocator::push_back()
{
	//assert(m_size != 0xff);
	if (m_size == 0xffff)
	{
		printf("%i\n", m_size);
		VX_ASSERT(false);
	}

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

u8* ChunkAllocator::allocate()
{
	if (m_size == 0 ||
		m_pChunks[m_allocChunk].freeBlocks == 0)
	{
		// no available memory
		/*auto i = m_pChunks;
		for (;; ++i)
		{
		if (i == (m_pChunks + m_size))
		{
		// add new chunk
		//auto sz = m_chunks.size();
		//m_chunks.reserve(sz + 1);

		//	Chunk newChunk;
		//newChunk.init(m_blockSize, m_blockCount);

		m_deallocChunk = m_allocChunk = push_back();
		break;
		}

		if (i->freeBlocks > 0)
		{
		// found chunk
		//m_allocChunk = (i - m_chunks.begin());
		m_allocChunk = (i - m_pChunks);
		break;
		}
		}*/

		bool found = false;
		for (auto i = 0u; i < m_size; ++i)
		{
			if (m_pChunks[i].freeBlocks > 0)
			{
				// found chunk
				m_allocChunk = i;
				found = true;
				break;
			}
		}

		if (!found)
		{
			// add new chunk
			m_deallocChunk = m_allocChunk = push_back();
		}
	}

	Chunk &allocChnk = m_pChunks[m_allocChunk];
	VX_ASSERT(allocChnk.freeBlocks > 0);
	return  allocChnk.allocate(m_blockSize);
}

// make sure p is not nullptr, and call contains() before calling deallocate
void ChunkAllocator::deallocate(u8* p)
{
	VX_ASSERT(p != nullptr);

	// we called contains() before calling deallocate, so this should be quick
	//bool found = (contains(p) != 0);

	//assert(found);
	VX_ASSERT(m_pChunks[m_deallocChunk].contains(p, m_blockSize, m_blockCount));

	m_pChunks[m_deallocChunk].deallocate(p, m_blockSize);
}

u8 ChunkAllocator::contains(u8* p)
{
	u8 found = 1;

	if (!m_pChunks[m_deallocChunk].contains(p, m_blockSize, m_blockCount))
	{
		found = 0;
		for (auto i = 0u; i < m_size; ++i)
		{
			auto &it = m_pChunks[i];
			if (it.contains(p, m_blockSize, m_blockCount))
			{
				m_deallocChunk = i;
				found = 1;
				break;
			}
		}
	}

	return found;
}

u32 SmallObjAllocator::createAllocator(u16 size)
{
	auto mod = m_chunkSize & size;
	auto blockCount = (m_chunkSize / size) + mod;
	blockCount = std::min(blockCount, 0xffffu);

	ChunkAllocator alloc;
	alloc.init(size, blockCount);

	auto index = m_allocators.size();
	m_allocators.push_back(std::move(alloc));

	return index;
}

void SmallObjAllocator::sortAllocators()
{
	std::sort(m_allocators.begin(), m_allocators.end(), [](const ChunkAllocator &l, const ChunkAllocator &r)
	{
		return l.getBlockSize() < r.getBlockSize();
	});
}


u8* SmallObjAllocator::allocate(u32 size)
{
	if (size > static_cast<u32>(s_maxObjSize))
	{
		return (u8*)::operator new(size);
	}

	std::lock_guard<std::mutex> guard(m_mutex);

	if (m_allocators.size() == 0 ||
		m_allocators[m_lastAlloc].getBlockSize() < size)
	{
		bool found = false;
		auto it = m_allocators.begin();
		while (it != m_allocators.end())
		{
			if (it->getBlockSize() >= size)
			{
				found = true;
				break;
			}

			++it;
		}

		if (!found)
		{
			// create new allocator
			m_lastDealloc = m_lastAlloc = createAllocator(size);

			sortAllocators();
		}
	}

	return m_allocators[m_lastAlloc].allocate();
}

bool SmallObjAllocator::deallocate(u8* p, u32 size)
{
	if (p == nullptr)
		return true;

	std::lock_guard<std::mutex> guard(m_mutex);

	if (!m_allocators[m_lastDealloc].contains(p))
	{
		auto iter = std::lower_bound(m_allocators.begin(), m_allocators.end(), size, [](const ChunkAllocator &l, u32 val)
		{
			return l.getBlockSize() < val;
		});

		bool found = false;
		while (iter != m_allocators.end())
		{
			if (iter->contains(p))
			{
				found = true;
				break;
			}

			++iter;
		}

		if (!found)
			return false;

		m_lastDealloc = iter - m_allocators.begin();
	}

	m_allocators[m_lastDealloc].deallocate(p);
	return true;
}