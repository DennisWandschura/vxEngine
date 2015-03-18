#include "PoolAllocator.h"
#include <algorithm>

namespace vx
{
	PoolAllocator::PoolAllocator() noexcept
		:m_pMemory(nullptr),
		m_firstFreeEntry(0),
		m_freeEntries(0),
		m_poolSize(0),
		m_size(0)
	{
	}

	PoolAllocator::PoolAllocator(U8 *ptr, U32 size, U32 poolSize, U8 alignment) noexcept
		:m_pMemory(ptr),
		m_firstFreeEntry(0),
		m_freeEntries(0),
		m_poolSize(poolSize),
		m_size(size)
	{
		auto adjustment = vx::AllocatorBase::getAdjustment(ptr, alignment);
		m_firstFreeEntry = adjustment;

		U8 *pCurrent = ptr + adjustment;
		poolSize = poolSize + vx::AllocatorBase::getAdjustment(pCurrent + poolSize, alignment);

		while (size >= poolSize)
		{
			auto pNext = pCurrent + poolSize;
			*reinterpret_cast<U32*>(pCurrent) = (pNext - ptr);

			pCurrent = pNext;

			++m_freeEntries;
			size -= poolSize;
		}
	}

	PoolAllocator::~PoolAllocator()
	{
		m_pMemory = nullptr;
		m_freeEntries = 0;
	}

	PoolAllocator& PoolAllocator::operator = (PoolAllocator &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_pMemory, rhs.m_pMemory);
			std::swap(m_firstFreeEntry, rhs.m_firstFreeEntry);
			std::swap(m_freeEntries, rhs.m_freeEntries);
			std::swap(m_poolSize, rhs.m_poolSize);
			std::swap(m_size, rhs.m_size);
		}
		return *this;
	}

	void* PoolAllocator::allocate(U32 size) noexcept
	{
		VX_UNREFERENCED_PARAMETER(size);

		if (m_freeEntries == 0)
			return nullptr;

		auto ptr = m_pMemory + m_firstFreeEntry;
		m_firstFreeEntry = *((U32*)ptr);
		--m_freeEntries;

		return ptr;
	}

		void* PoolAllocator::allocate(U32 size, U8 alignment) noexcept
	{
		VX_UNREFERENCED_PARAMETER(alignment);
		return allocate(size);
	}

		void PoolAllocator::deallocate(void *ptr)
	{
		if (ptr == nullptr ||
			ptr < m_pMemory ||
			m_pMemory + m_size <= ptr)
			return;

		U32 firstFreeEntry = ((U8*)ptr) - m_pMemory;
		*reinterpret_cast<U32*>(ptr) = m_firstFreeEntry;
		m_firstFreeEntry = firstFreeEntry;
		++m_freeEntries;
	}

	void* PoolAllocator::release() noexcept
	{
		auto ptr = m_pMemory;

		m_pMemory = nullptr;
		m_firstFreeEntry = 0;
		m_freeEntries = 0;
		m_poolSize = 0;
		m_size = 0;

		return ptr;
	}
}