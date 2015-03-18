#include "SmallObjAllocator.h"
#include <algorithm>

void Chunk::init(U32 blockSize, U16 blockCount)
{
	ptr = new U8[blockSize * blockCount];
	freeBlocks = blockCount;
	firstFreeBlock = 0;

	auto p = ptr;
	for (U32 i = 0; i < blockCount; p += blockSize)
	{
		*p = ++i;
	}
}

U8* Chunk::allocate(U32 blockSize)
{
	if (freeBlocks == 0)
		return nullptr;

	U8* p = ptr + (firstFreeBlock * blockSize);

	firstFreeBlock = *p;
	--freeBlocks;

	return p;
}

void Chunk::deallocate(U8* p, U32 blockSize)
{
	assert(p >= ptr);
	auto toRelease = p;
	assert((toRelease - ptr) % blockSize == 0);
	*toRelease = firstFreeBlock;
	firstFreeBlock = ((toRelease - ptr) / blockSize);
	assert(firstFreeBlock == (toRelease - ptr) / blockSize);

	++freeBlocks;
}

U8 Chunk::contains(U8* p, U32 blockSize, U8 blockCount)
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

void ChunkAllocator::init(U16 blockSize, U16 blockCount)
{
	m_blockSize = blockSize;
	m_blockCount = blockCount;
}

U8* ChunkAllocator::allocate()
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
	assert(allocChnk.freeBlocks > 0);
	return  allocChnk.allocate(m_blockSize);
}

// make sure p is not nullptr, and call contains() before calling deallocate
void ChunkAllocator::deallocate(U8* p)
{
	VX_ASSERT(p != nullptr, "");

	// we called contains() before calling deallocate, so this should be quick
	//bool found = (contains(p) != 0);

	//assert(found);
	VX_ASSERT(m_pChunks[m_deallocChunk].contains(p, m_blockSize, m_blockCount), "");

	m_pChunks[m_deallocChunk].deallocate(p, m_blockSize);
}

U8 ChunkAllocator::contains(U8* p)
{
	U8 found = 1;

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

U32 SmallObjAllocator::createAllocator(U16 size)
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


U8* SmallObjAllocator::allocate(U32 size)
{
	if (size > static_cast<U32>(s_maxObjSize))
	{
		return (U8*)::operator new(size);
	}

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

void SmallObjAllocator::deallocate(U8* p, U32 size)
{
	if (p == nullptr)
		return;

	if (!m_allocators[m_lastDealloc].contains(p))
	{
		/*auto sz = m_allocators.size();
		for (U32 i = 0; i < sz; ++i)
		{
		if (m_allocators[i].contains(p))
		{
		m_lastDealloc = i;
		break;
		}
		}*/

		auto iter = std::lower_bound(m_allocators.begin(), m_allocators.end(), size, [](const ChunkAllocator &l, U32 val)
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

		VX_ASSERT(found, "");
		m_lastDealloc = iter - m_allocators.begin();
	}

	m_allocators[m_lastDealloc].deallocate(p);
}