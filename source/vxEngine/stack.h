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

#include <vxLib\types.h>
#include <mutex>

template<class T, u32 N>
class stack
{
	T m_data[N];
	u32 m_size;
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