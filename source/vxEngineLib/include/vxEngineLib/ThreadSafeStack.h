#pragma once

#include <vxLib/types.h>
#include <atomic>

template<typename T>
class ThreadSafeStack
{
	typedef T value_type;

	value_type* m_ptr;
	std::atomic_uint m_size;
	u32 m_capacity;

public:
	ThreadSafeStack()
		:m_ptr(nullptr),
		m_size(),
		m_capacity(0)
	{
		m_size.store(0);
	}

	~ThreadSafeStack()
	{
		clear();
		m_ptr = 0;
		m_size.store(0);
		m_capacity = 0;
	}

	void initialize(u8* ptr, u32 capacity)
	{
		if (m_ptr == nullptr)
		{
			m_ptr = (value_type*)ptr;
			m_capacity = capacity;
		}
	}

	bool pushTS(const value_type &value)
	{
		auto index = m_size.fetch_add(1);
		if (index >= m_capacity)
		{
			m_size.store(m_capacity);
			return false;
		}

		new (&m_ptr[index]) value_type(value);
		return true;
	}

	bool pushTS(value_type &&value)
	{
		auto index = m_size.fetch_add(1);
		if (index >= m_capacity)
		{
			m_size.store(m_capacity);
			return false;
		}

		new (&m_ptr[index]) value_type(std::move(value));
		return true;
	}

	void clear()
	{
		auto sz = m_size.load();
		for (u32 i = 0; i < sz; ++i)
		{
			m_ptr[i].~value_type();
		}
		m_size.store(0);
	}

	u32 sizeTS() const
	{
		return m_size.load();
	}

	value_type& operator[](u32 i)
	{
		return m_ptr[i];
	}

	const value_type& operator[](u32 i) const
	{
		return m_ptr[i];
	}
};