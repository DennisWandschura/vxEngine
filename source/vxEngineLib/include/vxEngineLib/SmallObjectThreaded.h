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

class SmallObjAllocator;

#include <vector>
#include <vxEngineLib/SmallObjAllocator.h>

template<typename T>
class SmallObjectThreaded
{
	static std::vector<SmallObjAllocator*> s_allocators;
	static thread_local SmallObjAllocator* s_allocator;

public:
	static void setAllocator(SmallObjAllocator* allocator)
	{
		if (s_allocator == nullptr)
		{
			s_allocator = allocator;
			s_allocators.push_back(allocator);
		}
	}

	static void* operator new(std::size_t size)
	{
		return s_allocator->allocate(size);
	}

	static void operator delete(void* p, std::size_t size)
	{
		bool found = true;
		if (!s_allocator->deallocate((u8*)p, static_cast<u32>(size)))
		{
			found = false;

			for (auto &it : s_allocators)
			{
				if (it->deallocate((u8*)p, static_cast<u32>(size)))
				{
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			delete(p);
		}
	}
};

template<typename T>
std::vector<SmallObjAllocator*> SmallObjectThreaded<T>::s_allocators = {};

template<typename T>
thread_local SmallObjAllocator* SmallObjectThreaded<T>::s_allocator = nullptr;