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

#include <vxEngineLib/ArrayAllocator.h>
#include <vxEngineLib/Pool.h>
#include <vxLib/Container/sorted_array.h>
#include <vxLib/StringID.h>
#include <vxEngineLib/Reference.h>
#include <vxLib/Allocator/StackAllocator.h>
#include <mutex>

template<typename T>
class ResourceManager
{
	mutable std::mutex m_mutexData;
	vx::Pool<ReferenceCounted<T>> m_poolData;
	vx::sorted_array<vx::StringID, Reference<T>> m_sortedData;

	std::mutex m_mutexScratchAllocator;
	vx::StackAllocator m_scratchAllocator;

	std::mutex m_mutexDataAllocator;
	ArrayAllocator m_dataAllocator;

public:
	ResourceManager():m_mutexData(), m_poolData(), m_sortedData(), m_mutexScratchAllocator(), m_scratchAllocator(), m_mutexDataAllocator(), m_dataAllocator(){}
	~ResourceManager() {}

	void initialize(u16 maxCount, vx::StackAllocator* allocator)
	{
		auto sizeInBytes = sizeof(T) * maxCount;
		m_poolData.initialize(allocator->allocate(sizeInBytes, 64), maxCount);

		m_sortedData = vx::sorted_array<vx::StringID, Reference<T>>(maxCount, allocator);

		m_allocator.create(allocator->allocate(64 KBYTE, 4), 64 KBYTE);
	}

	void shutdown()
	{
		std::lock_guard<std::mutex> guard(m_mutexData);

		m_allocator.release();
		m_sortedData.cleanup();
		m_poolData.release();
	}

	void update()
	{
		std::unique_lock<std::mutex> lockData(m_mutexDataAllocator);
		m_dataAllocator.update(2);
	}

	template<typename ...Args>
	Reference<T> insertEntry(const vx::StringID &sid, Args&& ...args)
	{
		std::lock_guard<std::mutex> guard(m_mutexData);

		Reference<T> result;

		auto it = m_sortedData.find(sid);
		if (it != m_sortedData.end())
		{
			result = *it;
		}
		else
		{

			u16 index = 0xffff;
			auto ptr = m_poolData.createEntry(&index, std::forward<Args>(args)...);
			if (ptr != nullptr)
			{
				result = (*ptr);
				m_sortedData.insert(sid, result);
			}
		}

		return result;
	}

	vx::StackAllocator* lockScratchAllocator(std::unique_lock<std::mutex>* lock)
	{
		std::unique_lock<std::mutex> lck(m_mutexScratchAllocator);
		lock->swap(lck);

		return &m_scratchAllocator;
	}

	ArrayAllocator* lockDataAllocator(std::unique_lock<std::mutex>* lock)
	{
		std::unique_lock<std::mutex> lck(m_mutexDataAllocator);
		lock->swap(lck);

		return &m_dataAllocator;
	}

	Reference<T> find(const vx::StringID &sid) const
	{
		Reference<T> result;

		std::lock_guard<std::mutex> guard(m_mutexData);
		auto it = m_sortedData.find(sid);
		if (it != m_sortedData.end())
		{
			result = *it;
		}

		return result;
	}

	vx::StackAllocator* getScratchAllocator()
	{
		return &m_scratchAllocator;
	}

	std::mutex* getScratchAllocatorMutex()
	{
		return &m_mutexScratchAllocator;
	}
};