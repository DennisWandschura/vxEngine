#pragma once

#include <vxLib/types.h>

template<typename T, U32 SIZE>
class RingBuffer
{
	T m_data[SIZE];
	U32 m_head;
	U32 m_tail;

public:
	void push(const T &value)
	{
		assert((m_tail + 1) % SIZE != m_head);

		// Add to the end of the list.
		m_data[m_tail] = value;
		m_tail = (m_tail + 1) % SIZE;
	}

	void pop(T &value)
	{
		value = std::move(m_data[m_tail]);
		--m_tail;
	}
};