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

#include <vxEngineLib/managed_ptr.h>
#include <vxEngineLib/ArrayAllocator.h>

template<typename T, typename ...Args>
typename std::enable_if<!std::is_array<T>::value, managed_ptr<T>>::type
createPtr(ArrayAllocator* alloc, Args&& ...args)
{
	managed_ptr<T> ptr = alloc->allocate(sizeof(T), __alignof(T));

	if (ptr.get() != nullptr)
	{
		new (ptr.get()) T(std::forward<Args>(args)...);
	}

	return ptr;
}

template<typename T>
typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0,
	managed_ptr<T, DefaultDeleter<T>> >::type createPtr(ArrayAllocator* alloc, unsigned int count)
{
	typedef typename std::remove_extent<T>::type _Elem;

	managed_ptr<T, DefaultDeleter<T>> ptr = alloc->allocate(sizeof(_Elem) * count, __alignof(_Elem));

	auto p = ptr.get();
	if (p != nullptr)
	{
		auto last = p + count;
		while (p != last)
		{
			new (p)T{};
			++p;
		}
	}

	return ptr;
}