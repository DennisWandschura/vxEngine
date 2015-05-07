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

#include <vxLib/types.h>
#include <new>

class PoolBase
{
protected:
	u8* m_ptr{ nullptr };
	u32 m_firstFreeEntry{ 0 };
	u16 m_freeEntries{ 0 };
	u16 m_capacity{ 0 };

	PoolBase(){}
	virtual ~PoolBase(){}

	void initialize(u8* ptr, u16 capacity, u32 chunkSize);

	u8* createEntry(u16* index, u32 chunkSize);

	// does no checking
	void destroyEntry(u8* ptr, u16 index);

	// checks if the entry at ptr is not used and valid
	void validateEmptyEntry(const u8 *ptr);

	// checks if the entry at ptr is really used
	void validateUsedEntry(const u8 *ptr);

	bool isUsed(const u8* ptr) const;

	// returns first used entry or nullptr
	u8* first(u32 chunkSize) const;

	// returns ptr to next item or nullptr, does no checking
	u8* next(u8* ptr, u32 chunkSize) const;

	void* operator[](u16 i);
	const void* operator[](u16 i) const;

	void* get(){ return m_ptr; }
	const void* get() const{ return m_ptr; }

public:
	u8* release();

	u16 size() const;
};

template<typename T>
class Pool : public PoolBase
{
	using value_type = T;
	using reference = T&;
	using const_reference = const reference;
	using pointer = value_type*;

public:
	void initialize(u8* ptr, u16 capacity)
	{
		assert(ptr && capacity != 0);
		PoolBase::initialize(ptr, capacity, sizeof(T));
	}

	pointer createEntry(u16* index)
	{
		auto p = PoolBase::createEntry(index, sizeof(T));

		new ((void*)p) value_type();

		return reinterpret_cast<pointer>(p);
	}

	template<typename ...Args>
	pointer createEntry(u16* index, Args&& ...args)
	{
		auto p = PoolBase::createEntry(index, sizeof(T));

		new ((void*)p) value_type(std::forward<Args>(args)...);

		return reinterpret_cast<pointer>(p);
	}

	void destroyEntry(pointer ptr)
	{
		if (ptr == nullptr || 
			ptr < (T*)m_ptr ||
			((T*)m_ptr) + m_capacity <= ptr)
			return;


		validateUsedEntry(reinterpret_cast<u8*>(ptr));
		ptr->~T();

		auto index = ptr - reinterpret_cast<pointer>(m_ptr);

		PoolBase::destroyEntry((u8*)ptr, index);
	}

	reference operator[](u16 i)
	{
		return reinterpret_cast<pointer>(m_ptr)[i];
	}

	const_reference operator[](u16 i) const
	{
		return reinterpret_cast<pointer>(m_ptr)[i];
	}

	pointer get() { return (pointer)PoolBase::get(); }
	const pointer get() const { return (const pointer)PoolBase::get(); }

	pointer first() const
	{
		return (pointer)PoolBase::first(sizeof(T));
	}

	pointer next_nocheck(pointer current) const
	{
		return (pointer)PoolBase::next((u8*)current, sizeof(T));
	}

	u16 getIndex_nocheck(pointer p) const
	{
		auto index = (p - reinterpret_cast<pointer>(m_ptr));
		return index;
	}

	void clear()
	{
		auto current = first();
		while (current != nullptr)
		{
			current->~value_type();
			current = next_nocheck(current);
		}
	}
};