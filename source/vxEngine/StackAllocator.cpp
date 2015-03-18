#include "StackAllocator.h"
namespace vx
{
	StackAllocator::StackAllocator()
		:m_pMemory(nullptr),
		m_head(0),
		m_size(0)
	{

	}

	StackAllocator::StackAllocator(U8 *ptr, U32 size)
		:m_pMemory(ptr),
		m_head(0),
		m_size(size)
	{
		VX_ASSERT(ptr != nullptr, "Passed nullptr !");

		auto adjustment = vx::AllocatorBase::getAdjustment(m_pMemory, 16);
		VX_ASSERT(adjustment == 0, "Memory not properly aligned !");
	}

	StackAllocator::~StackAllocator()
	{
		m_pMemory = nullptr;
		m_head = 0;
		m_size = 0;
	}

	StackAllocator& StackAllocator::operator = (StackAllocator &&rhs)
	{
		if (this != &rhs)
		{
			this->swap(rhs);
		}
		return *this;
	}

	U8* StackAllocator::allocate(U32 size) noexcept
	{
		U8 *ptr = nullptr;

		if (m_head + size <= m_size)
		{
			ptr = m_pMemory + m_head;
			m_head += size;
		}

		return ptr;
	}

		U8* StackAllocator::allocate(U32 size, U8 alignment) noexcept
	{
		U8 *ptr = nullptr;

		auto pHead = m_pMemory + m_head;
		auto adjustment = vx::AllocatorBase::getAdjustment(pHead, alignment);
		auto adjustedSize = size + adjustment;

		if (m_head + adjustedSize <= m_size)
		{
			ptr = pHead + adjustment;
			m_head += adjustedSize;
		}

		return ptr;
	}

		void StackAllocator::deallocate(void*) noexcept
	{
	}

	void StackAllocator::clear() noexcept
	{
		m_head = 0;
	}

		void StackAllocator::clear(Marker marker) noexcept
	{
		if (m_head >= marker)
		m_head = marker;
	}

		void* StackAllocator::release() noexcept
	{
		auto ptr = m_pMemory;

		m_pMemory = nullptr;
		m_head = 0;
		m_size = 0;

		return ptr;
	}

		void StackAllocator::swap(StackAllocator &rhs) noexcept
	{
		auto pMemory = m_pMemory;
		auto head = m_head;
		auto size = m_size;

		m_pMemory = rhs.m_pMemory;
		m_head = rhs.m_head;
		m_size = rhs.m_size;

		rhs.m_pMemory = pMemory;
		rhs.m_head = head;
		rhs.m_size = size;
	}

		StackAllocator::Marker StackAllocator::getMarker() const
	{
		return m_head;
	}
}