/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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

	u8 try_push_back(const value_type &val)
	{
		u8 result = 0;
		std::unique_lock<std::mutex> lck(m_mutex, std::defer_lock);

		if (lck.try_lock())
		{
			result = 1;
			m_data.push_back(val);
		}

		return result;
	}

	u8 try_pop_back(value_type &value)
	{
		std::unique_lock<std::mutex> lck(m_mutex, std::defer_lock);

		u8 result = 0;
		if (lck.try_lock() &&
			m_data.size() != 0)
		{
			value = std::move(m_data.back());
			m_data.pop_back();
			result = 1;
		}

		return result;
	}

	void reserve(u32 n)
	{
		std::lock_guard<std::mutex> lck(m_mutex);
		m_data.reserve(n);
	}
};