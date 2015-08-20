#pragma once

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

#include <vxLib/type_traits.h>

namespace vx
{
	template<typename T>
	struct shared_ptr
	{
		T* m_ptr;

		void increment()
		{
			if (m_ptr)
			{
				m_ptr->increment();
			}
		}

		void cleanup()
		{
			if (m_ptr)
			{
				auto refCount = m_ptr->decrement() - 1;

				if (refCount == 0)
				{
					delete(m_ptr);
				}
			}
		}

	public:
		shared_ptr() :m_ptr(nullptr) {}
		shared_ptr(T* p) :m_ptr(p) { increment(); }

		shared_ptr(const shared_ptr &rhs)
			:m_ptr(rhs.m_ptr)
		{
			increment();
		}

		shared_ptr(shared_ptr &&rhs)
			:m_ptr(rhs.m_ptr)
		{
			rhs.m_ptr = nullptr;
		}

		~shared_ptr()
		{
			cleanup();
			m_ptr = nullptr;
		}

		shared_ptr& operator=(const shared_ptr &rhs)
		{
			if (this != &rhs)
			{
				cleanup();

				m_ptr = rhs.m_ptr;
				increment();
			}
			return *this;
		}

		shared_ptr& operator=(shared_ptr &&rhs)
		{
			if (this != &rhs)
			{
				auto tmp = m_ptr;
				m_ptr = rhs.m_ptr;
				rhs.m_ptr = tmp;
			}
			return *this;
		}

		void swap(shared_ptr &other)
		{
			auto tmp = m_ptr;
			m_ptr = other.m_ptr;
			other.m_ptr = tmp;
		}

		T* operator->()
		{
			return m_ptr;
		}

		const T* operator->() const
		{
			return m_ptr;
		}

		T* get() { return m_ptr; }

		const T* get() const { return m_ptr; }

		operator bool()
		{
			return (m_ptr != nullptr);
		}
	};
}