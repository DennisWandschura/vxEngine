#pragma once

#include <vxLib\types.h>
#include <mutex>

template<class T, U32 N>
class stack
{
	T m_data[N];
	U32 m_size;
	std::mutex m_mutex;

public:
	stack() :m_size(0), m_mutex()
	{
	}

	bool try_push_back(const T &value)
	{
		std::unique_lock<std::mutex> lck(m_mutex, std::defer_lock);
		if (!lck.try_lock())
			return false;

		if (m_size >= N)
			return false;

		m_data[m_size++] = value;

		return true;
	}

	bool try_pop_back(T &value)
	{
		std::unique_lock<std::mutex> lck(m_mutex, std::defer_lock);

		if (!lck.try_lock())
			return false;

		if (m_size == 0)
			return false;

		value = std::move(m_data[--m_size]);

		return true;
	}
};