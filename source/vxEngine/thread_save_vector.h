#pragma once

#include <vector>
#include <mutex>

template<typename T>
class thread_safe_vector
{
	using value_type = T;

	std::mutex m_mutex;
	std::vector<T> m_data;

public:
	thread_safe_vector()
		:m_mutex(),
		m_data()
	{
	}

	U8 try_push_back(const value_type &val)
	{
		U8 result = 0;
		std::unique_lock<std::mutex> lck(m_mutex, std::defer_lock);

		if (lck.try_lock())
		{
			result = 1;
			m_data.push_back(val);
		}

		return result;
	}

	U8 try_pop_back(value_type &value)
	{
		std::unique_lock<std::mutex> lck(m_mutex, std::defer_lock);

		U8 result = 0;
		if (lck.try_lock() &&
			m_data.size() != 0)
		{
			value = std::move(m_data.back());
			m_data.pop_back();
			result = 1;
		}

		return result;
	}

	void reserve(U32 n)
	{
		std::lock_guard<std::mutex> lck(m_mutex);
		m_data.reserve(n);
	}
};