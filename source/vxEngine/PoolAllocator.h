#pragma once

#include <vxLib/Allocator/Allocator.h>

namespace vx
{
	class PoolAllocator : public AllocatorBase
	{
		U8 *m_pMemory;
		U32 m_firstFreeEntry;
		U32 m_freeEntries;
		U32 m_poolSize;
		U32 m_size;

	public:
		PoolAllocator() noexcept;
		PoolAllocator(U8 *ptr, U32 size, U32 poolSize, U8 alignment) noexcept;
		PoolAllocator(const PoolAllocator&) = delete;
		PoolAllocator(PoolAllocator &&rhs);
		~PoolAllocator();

		PoolAllocator& operator=(const PoolAllocator&) = delete;
		PoolAllocator& operator=(PoolAllocator &&rhs);

		// ignores size parameter
		void* allocate(U32 size) noexcept;
		// ignores size and alginment parameter
		void* allocate(U32 size, U8 alignment) noexcept;

		void deallocate(void *ptr);

		// returns pointer to memory and sets everything to zero
		void* release() noexcept;
	};
}