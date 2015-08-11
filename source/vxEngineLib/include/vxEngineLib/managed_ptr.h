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
#include <utility>

template<typename T>
struct DefaultDeleter
{
	void operator()(T* p)
	{
		p->~T();
	}
};

template<typename T>
struct DefaultDeleter<T[]>
{
	typedef typename std::remove_extent<T>::type _Elem;

	void operator()(_Elem* p, size_t count)
	{
		auto last = p + count;
		while (p < last)
		{
			p->~T();
			++p;
		}
	}
};

class managed_ptr_base
{
	friend class ArrayAllocator;

protected:
	u8* m_ptr;
	ArrayAllocator* m_alloc;
	unsigned m_entryIndex;

	void swap(managed_ptr_base &other);

	void setPtr(u8* ptr)
	{
		this->m_ptr = ptr;
	}

	unsigned getEntryIndex() const
	{
		return m_entryIndex;
	}

	void updateAllocator();

	unsigned getEntrySize() const;

	managed_ptr_base();

	managed_ptr_base(const managed_ptr_base&) = delete;

	managed_ptr_base(managed_ptr_base &&rhs);

	managed_ptr_base(u8* p, ArrayAllocator* alloc, unsigned index);

	managed_ptr_base& operator=(const managed_ptr_base&) = delete;
	managed_ptr_base& operator=(managed_ptr_base &&rhs);
};

template<typename T, typename _Deleter = DefaultDeleter<T>>
class managed_ptr : public managed_ptr_base
{
	typedef managed_ptr_base MyBase;
	typedef _Deleter MyDeleter;

public:
	managed_ptr() :MyBase(){}
	managed_ptr(MyBase &&rhs): MyBase(std::move(rhs)){}
	managed_ptr(managed_ptr &&rhs) :MyBase(std::move(rhs)){}
	managed_ptr(const managed_ptr&) = delete;

	~managed_ptr()
	{
		clear();
	}

	managed_ptr& operator=(const managed_ptr&) = delete;

	managed_ptr& operator=(managed_ptr &&rhs)
	{
		MyBase::operator=(std::move(rhs));

		return *this;
	}

	void swap(managed_ptr &other)
	{
		MyBase::swap(other);
	}

	T* get()
	{
		return (T*)m_ptr;
	}

	const T* get() const
	{
		return (T*)m_ptr;
	}

	void clear()
	{
		if (m_ptr)
		{
			auto p = get();
			MyDeleter()(p);
			alloc->deallocate(this);

			m_ptr = nullptr;
			alloc = nullptr;
		}
	}
};

template<typename T, typename _Deleter>
class managed_ptr<T[], _Deleter> : public managed_ptr_base
{
	typedef managed_ptr_base MyBase;
	typedef _Deleter MyDeleter;

public:
	managed_ptr() :MyBase(){}
	managed_ptr(MyBase &&rhs) : MyBase(std::move(rhs)){}
	managed_ptr(managed_ptr &&rhs) :MyBase(std::move(rhs)){}
	managed_ptr(const managed_ptr&) = delete;

	~managed_ptr()
	{
		clear();
	}

	managed_ptr& operator=(const managed_ptr&) = delete;

	managed_ptr& operator=(managed_ptr &&rhs)
	{
		MyBase::operator=(std::move(rhs));

		return *this;
	}

	void swap(managed_ptr &other)
	{
		MyBase::swap(other);
	}

	T* get()
	{
		return (T*)m_ptr;
	}

	const T* get() const
	{
		return (T*)m_ptr;
	}

	void clear()
	{
		if (m_ptr)
		{
			auto count = getEntrySize() / sizeof(T);
			auto p = get();
			MyDeleter()(p, count);

			m_alloc->deallocate(this);

			m_ptr = nullptr;
			m_alloc = nullptr;
		}
	}

	T& operator[](unsigned i)
	{
		return get()[i];
	}

	const T& operator[](unsigned i) const
	{
		return get()[i];
	}
};