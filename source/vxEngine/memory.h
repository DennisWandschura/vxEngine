#pragma once

#include <vxLib/types.h>

class Memory
{
	U8* m_pMemory{nullptr};
	size_t m_size;
	size_t m_alignment;

public:
	Memory(){}

	Memory(size_t size, size_t alignment)
		:m_size(size),
		m_alignment(alignment)
	{
		m_pMemory = (U8*)_aligned_malloc(size, alignment);
		assert(m_pMemory);
	}

	Memory(const Memory&) = delete;

	Memory(Memory &&rhs)
		:m_pMemory(rhs.m_pMemory),
		m_size(rhs.m_size),
		m_alignment(rhs.m_alignment)
	{
		rhs.m_pMemory = nullptr;
		rhs.m_size = 0;
	}

	~Memory()
	{
		_aligned_free(m_pMemory);
	}

	Memory& operator=(const Memory&) = delete;

	Memory& operator=(Memory &&rhs)
	{
		if (this != &rhs)
		{
			this->swap(rhs);
		}
		return *this;
	}

	void swap(Memory &other)
	{
		std::swap(m_pMemory, other.m_pMemory);
		std::swap(m_size, other.m_size);
		std::swap(m_alignment, other.m_alignment);
	}

	U8* get()
	{
		return m_pMemory;
	}

	const U8* get() const
	{
		return m_pMemory;
	}

	size_t size() const
	{
		return m_size;
	}

	size_t alignment() const
	{
		return m_alignment;
	}
};