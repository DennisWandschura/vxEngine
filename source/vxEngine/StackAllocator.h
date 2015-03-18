#ifndef __VX_STACKALLOCATOR_H
#define __VX_STACKALLOCATOR_H
#pragma once

#include <vxLib/Allocator/Allocator.h>

namespace vx
{
	class StackAllocator : public AllocatorBase
	{
		U8 *m_pMemory;
		U32 m_head;
		U32 m_size;

	public:
		typedef U32 Marker;

		StackAllocator();
		// passed in memory ptr must be aligned to 16 bytes
		StackAllocator(U8 *ptr, U32 size);
		StackAllocator(const StackAllocator&) = delete;
		StackAllocator(StackAllocator &&rhs);
		~StackAllocator();

		StackAllocator& operator=(const StackAllocator&) = delete;
		StackAllocator& operator=(StackAllocator &&rhs);

		U8* allocate(U32 size) noexcept;
		U8* allocate(U32 size, U8 alignment) noexcept;

		void deallocate(void *ptr) noexcept;

		void clear() noexcept;
		void clear(Marker marker) noexcept;

		void* release() noexcept;

		void swap(StackAllocator &rhs) noexcept;

		Marker getMarker() const;
	};
}
#endif