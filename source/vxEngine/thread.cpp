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
#include "thread.h"

namespace vx
{
	thread::thread()
		:m_thread()
	{

	}

	thread::thread(std::thread &&t)
		: m_thread(std::move(t))
	{

	}

	thread::thread(thread &&rhs)
		: m_thread(std::move(rhs.m_thread))
	{

	}

	thread::~thread()
	{
		join();
	}

	thread& thread::operator = (std::thread &&rhs)
	{
		m_thread = std::move(rhs);
		return *this;
	}

	thread& thread::operator = (thread &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_thread, rhs.m_thread);
		}

		return *this;
	}

	U32 thread::hardware_concurrency() noexcept
	{
		return std::thread::hardware_concurrency();
	}

	bool thread::joinable() const noexcept
	{
		return m_thread.joinable();
	}

	void thread::detach() noexcept
	{
		if (m_thread.joinable())
		{
			m_thread.detach();
		}
	}

	void thread::join() noexcept
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	std::thread::id thread::get_id() const noexcept
	{
		return m_thread.get_id();
	}
}