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

#include <vxLib/types.h>
#include <atomic>

template<typename T>
class ReferenceCounted
{
	T m_data;
	std::atomic_uint32_t m_refCount{ 0 };

public:
	u32 increment()
	{
		return ++m_refCount;
	}

	u32 decrement()
	{
		return --m_refCount;
	}

	u32 getRefCount() const
	{
		return m_refCount.load();
	}

	operator T&()
	{
		return m_data;
	}

	operator const T&() const
	{
		return m_data;
	}
};

template<typename T>
class Reference
{
	ReferenceCounted<T>* m_ptr;

	void increment()
	{
		if (m_ptr)
			m_ptr->increment();
	}

	void decrement()
	{
		if (m_ptr)
		{
			auto refCount = m_ptr->decrement();
			if (refCount == 0)
			{
				delete(m_ptr);
			}
		}
	}

public:
	Reference() :m_ptr(nullptr){}

	Reference(const ReferenceCounted<T> &ref) :m_ptr(&ref){ increment(); }

	Reference(const Reference &rhs)
		:m_ptr(rhs.m_ptr)
	{
		increment();
	}

	Reference(Reference &&rhs)
		:m_ptr(rhs.m_ptr)
	{
		rhs.m_ptr = nullptr;
	}

	~Reference()
	{
		decrement();
		m_ptr = nullptr;
	}

	Reference& operator=(const Reference &rhs)
	{
		if (this != &rhs)
		{
			decrement();
			m_ptr = rhs.m_ptr;
			increment();
		}

		return *this;
	}

	Reference& operator=(Reference &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_ptr, rhs.m_ptr);
		}

		return *this;
	}

	typename std::add_reference<T>::type operator*() const
	{	// return reference to object
		return (*this->m_ptr);
	}

	T* operator->()
	{
		return m_ptr;
	}
};