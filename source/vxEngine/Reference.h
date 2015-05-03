#pragma once

#include <vxLib/types.h>
#include <atomic>

template<typename T>
class ReferenceCounted
{
	T m_data;
	std::atomic_uint32_t m_refCount{ 0 };

public:
	U32 increment()
	{
		return ++m_refCount;
	}

	U32 decrement()
	{
		return --m_refCount;
	}

	U32 getRefCount() const
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