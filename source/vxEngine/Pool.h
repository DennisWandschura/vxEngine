#pragma once

#include <vxLib/types.h>
#include <new>

class PoolBase
{
protected:
	U8* m_ptr{ nullptr };
	U32 m_firstFreeEntry{ 0 };
	U16 m_freeEntries{ 0 };
	U16 m_capacity{ 0 };

	PoolBase(){}
	~PoolBase(){}

	void initialize(U8* ptr, U16 capacity, U32 chunkSize);

	U8* createEntry(U16* index, U32 chunkSize);

	// does no checking
	void destroyEntry(U8* ptr, U16 index);

	// checks if the entry at ptr is not used and valid
	void validateEmptyEntry(const U8 *ptr);

	// checks if the entry at ptr is really used
	void validateUsedEntry(const U8 *ptr);

	bool isUsed(const U8* ptr) const;

	// returns first used entry or nullptr
	U8* first(U32 chunkSize) const;

	// returns ptr to next item or nullptr, does no checking
	U8* next(U8* ptr, U32 chunkSize) const;

	void* operator[](U16 i);
	const void* operator[](U16 i) const;

	void* get(){ return m_ptr; }
	const void* get() const{ return m_ptr; }

public:
	U8* release();

	U16 size() const;
};

template<typename T>
class Pool : public PoolBase
{
	using value_type = T;
	using reference = T&;
	using const_reference = const reference;
	using pointer = value_type*;

public:
	void initialize(U8* ptr, U16 capacity)
	{
		assert(ptr && capacity != 0);
		PoolBase::initialize(ptr, capacity, sizeof(T));
	}

	pointer createEntry(U16* index)
	{
		auto p = PoolBase::createEntry(index, sizeof(T));

		new ((void*)p) value_type();

		return reinterpret_cast<pointer>(p);
	}

	template<typename ...Args>
	pointer createEntry(U16* index, Args&& ...args)
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


		validateUsedEntry(reinterpret_cast<U8*>(ptr));
		ptr->~T();

		auto index = ptr - reinterpret_cast<pointer>(m_ptr);

		PoolBase::destroyEntry((U8*)ptr, index);
	}

	reference operator[](U16 i)
	{
		return reinterpret_cast<pointer>(m_ptr)[i];
	}

	const_reference operator[](U16 i) const
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
		return (pointer)PoolBase::next((U8*)current, sizeof(T));
	}

	U16 getIndex_nocheck(pointer p) const
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