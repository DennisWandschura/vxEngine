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

template<typename T>
struct shared_ptr
{
	T* ptr;

	void increment()
	{
		if (ptr)
		{
			ptr->increment();
		}
	}

	void cleanup()
	{
		if (ptr)
		{
			auto refCount = ptr->decrement();
			if (refCount < 0)
			{
				puts("errr");
			}

			if (refCount <= 0)
			{
				delete(ptr);
			}
		}
	}

	shared_ptr() :ptr(nullptr) {}
	shared_ptr(T* p) :ptr(p) { increment(); }
	shared_ptr(const shared_ptr &rhs)
		:ptr(rhs.ptr)
	{
		increment();
	}

	shared_ptr(shared_ptr &&rhs)
		:ptr(rhs.ptr)
	{
		rhs.ptr = nullptr;
	}

	~shared_ptr()
	{
		cleanup();

		ptr = nullptr;
	}

	shared_ptr& operator=(const shared_ptr &rhs)
	{
		if (this != &rhs)
		{
			cleanup();
			ptr = rhs.ptr;
			increment();
		}

		return *this;
	}

	shared_ptr& operator=(shared_ptr &&rhs)
	{
		if (this != &rhs)
		{
			auto tmp = ptr;
			ptr = rhs.ptr;
			rhs.ptr = tmp;
		}

		return *this;
	}

	T* operator->()
	{
		return ptr;
	}

	const T* operator->() const
	{
		return ptr;
	}
};