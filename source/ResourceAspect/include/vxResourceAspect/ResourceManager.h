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
	vx::Pool<T> m_poolData;
	vx::sorted_array<vx::StringID, T*> m_sortedData;

	std::mutex m_mutexScratchAllocator;
	vx::StackAllocator m_scratchAllocator;

	std::mutex m_mutexDataAllocator;
	ArrayAllocator m_dataAllocator;

public:
	ResourceManager():m_mutexData(), m_poolData(), m_sortedData(), m_mutexScratchAllocator(), m_scratchAllocator(), m_mutexDataAllocator(), m_dataAllocator(){}
	~ResourceManager() {}

	bool initialize(u16 capacity, u32 dataSizeInBytes, u32 scratchSizeInBytes, vx::StackAllocator* allocator)
	{
		auto sizeInBytes = sizeof(T) * capacity;
		auto poolPtr = allocator->allocate(sizeInBytes, 64);
		if (poolPtr == nullptr)
			return false;

		m_poolData.initialize(poolPtr, capacity);

		m_sortedData = vx::sorted_array<vx::StringID, T*>(capacity, allocator);

		auto scratchPtr = allocator->allocate(scratchSizeInBytes, 4);
		if (scratchPtr == nullptr)
			return false;
		m_scratchAllocator = vx::StackAllocator(scratchPtr, scratchSizeInBytes);

		if (dataSizeInBytes != 0)
		{
			auto dataPtr = allocator->allocate(dataSizeInBytes, 4);
			if (dataPtr == nullptr)
				return false;

			m_dataAllocator.create(dataPtr, dataSizeInBytes);
		}

		return true;
	}

	void shutdown()
	{
		std::lock_guard<std::mutex> guard(m_mutexDataAllocator);
		m_dataAllocator.release();

		std::lock_guard<std::mutex> guard1(m_mutexScratchAllocator);
		m_scratchAllocator.release();

		std::lock_guard<std::mutex> guard2(m_mutexData);
		m_sortedData.cleanup();
		m_poolData.release();
	}

	u32 getMarker()
	{
		return m_scratchAllocator.getMarker();
	}

	void clearScratchAllocator()
	{
		m_scratchAllocator.clear();
	}

	void clearScratchAllocator(u32 marker)
	{
		m_scratchAllocator.clear(marker);
	}

	void update()
	{
		std::unique_lock<std::mutex> lockData(m_mutexDataAllocator);
		m_dataAllocator.update(2);
	}

	template<typename ...Args>
	T* insertEntry(const vx::StringID &sid, Args&& ...args)
	{
		std::lock_guard<std::mutex> guard(m_mutexData);

		T* result = nullptr;

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
				result = ptr;
				m_sortedData.insert(sid, ptr);
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

	T* find(const vx::StringID &sid) const
	{
		T* result = nullptr;

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