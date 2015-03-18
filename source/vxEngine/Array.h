#ifndef __VX_ARRAY_H
#define __VX_ARRAY_H
#pragma once

#include <vxLib/Container/iterator.h>
#include "StackAllocator.h"

namespace vx
{
	/*
	Array can hold a fixed amount of values.

	The memory is allocated from a Stack Allocator.
	Because a Stack Allocator does nothing on "deallocate()", the memory needs to be deallocated manually outside of the Array class.
	This is because we can not call StackAllocator::clear(), some other Container might be using the same Allocator and this might deallocate memory that its using.
	*/
	template<typename T>
	class Array
	{
	public:
		using _MyContainer= Array<T>;
		using value_type = T;
		using reference = value_type&;
		using const_reference= const reference;
		using pointer = value_type*;
		using const_pointer = const pointer;
		using MyAllocator = StackAllocator;

		using const_iterator = vx::vector_const_iterator<_MyContainer>;
		using iterator = vx::vector_iterator<_MyContainer>;

		using size_type = U32;
		using difference_type = size_t;

	private:
		pointer m_pValues{ nullptr };
		size_type m_size{ 0 };
		size_type m_capacity{ 0 };
		MyAllocator* m_pAllocator{ nullptr };

	public:
		// destroys all values and sets everything to zero
		void cleanup()
		{
			if (m_pValues && m_pAllocator)
			{
				MyAllocator::rangeDestroy(m_pValues, m_pValues + m_size);
				m_size = 0;
				m_capacity = 0;
				m_pValues = nullptr;

				// this does nothing really
				m_pAllocator->deallocate(m_pValues);
				m_pAllocator = nullptr;
			}
		}

		Array(){}

		// aquires memory from the allocator, make sure the allocator has enough space before calling this
		Array(size_type capacity, MyAllocator* pAllocator)
			:m_capacity(capacity),
			m_pAllocator(pAllocator)
		{
			m_pValues = (pointer)m_pAllocator->allocate(sizeof(value_type) * capacity, __alignof(value_type));
			VX_ASSERT(m_pValues);
		}

		Array(const Array&) = delete;

		Array& operator=(const Array&) = delete;

		~Array()
		{
			cleanup();
		}

		bool push_back(const value_type &value)
		{
			if (m_capacity <= m_size)
				return false;

			MyAllocator::construct(m_pValues + m_size, value);

			++m_size;

			return true;
		}

		bool push_back(value_type &&value)
		{
			if (m_capacity <= m_size)
				return false;

			MyAllocator::construct(m_pValues + m_size, std::forward<value_type>(value));

			++m_size;

			return true;
		}

		void pop_back()
		{
			vx::AllocatorBase::destroy(&back());
			--m_size;
		}

		template<typename... _Valty>
		bool emplace_back(_Valty&&... _Val)
		{
			if (m_capacity <= m_size)
				return false;

			MyAllocator::construct(m_pValues + m_size, std::forward<_Valty>(_Val)...);

			++m_size;

			return true;
		}

		void clear()
		{
			MyAllocator::rangeDestroy(m_pValues, m_pValues + m_size);
			m_size = 0;
		}

		reference front()
		{
			return *m_pValues;
		}

		const_reference front() const
		{
			return *m_pValues;
		}

		reference back()
		{
			return *(m_pValues + m_size - 1);
		}

		const_reference back() const
		{
			return *(m_pValues + m_size - 1);
		}

		const_iterator begin() const
		{
			return const_iterator(m_pValues, this);
		}

		iterator begin()
		{
			return iterator(m_pValues, this);
		}

		const_iterator end() const
		{
			return const_iterator(m_pValues + m_size, this);
		}

		iterator end()
		{
			return iterator(m_pValues + m_size, this);
		}

		size_type size() const
		{
			return m_size;
		}

		size_type capacity() const
		{
			return m_capacity;
		}
	};

	template<typename T>
	class ManagedArray
	{
	public:
		using _MyContainer = Array<T>;
		using value_type = T;
		using reference = value_type&;
		using const_reference = const reference;
		using pointer = value_type*;
		using const_pointer = const pointer;

		using const_iterator = vx::vector_const_iterator<_MyContainer>;
		using iterator = vx::vector_iterator<_MyContainer>;

		using size_type = U32;
		using difference_type = size_t;

	private:
		pointer m_pValues{ nullptr };
		size_type m_size{ 0 };
		size_type m_capacity{ 0 };

	public:
		// destroys all values and sets everything to zero
		pointer release()
		{
			auto p = m_pValues;
			if (m_pValues)
			{
				AllocatorBase::rangeDestroy(m_pValues, m_pValues + m_size);
				m_size = 0;
				m_pValues = nullptr;
				m_capacity = 0;
			}
			return p;
		}

		ManagedArray(){}

		template<typename Alloc>
		ManagedArray(size_type capacity, Alloc* pAllocator)
			:m_capacity(capacity)
		{
			m_pValues = (pointer)pAllocator->allocate(sizeof(value_type) * capacity);
			VX_ASSERT(m_pValues);
		}

		ManagedArray(const ManagedArray&) = delete;

		ManagedArray(ManagedArray &&rhs)
			:m_pValues(rhs.m_pValues),
			m_size(rhs.m_size),
			m_capacity(rhs.m_capacity)
		{
			rhs.m_pValues = nullptr;
			rhs.m_size = 0;
			rhs.m_capacity = 0;
		}

		ManagedArray& operator=(const ManagedArray&) = delete;

		ManagedArray& operator=(ManagedArray &&rhs)
		{
			if (this != &rhs)
			{
				this->swap(rhs);
			}
			return *this;
		}

		~ManagedArray()
		{
			release();
		}

		void swap(ManagedArray &other)
		{
			std::swap(m_pValues, other.m_pValues);
			std::swap(m_size, other.m_size);
			std::swap(m_capacity, other.m_capacity);
		}

		void push_back(const value_type &value)
		{
			VX_ASSERT(m_size < m_capacity);
			AllocatorBase::construct(m_pValues + m_size, value);
			++m_size;
		}

		template<typename = typename std::enable_if_t<std::is_nothrow_move_constructible<value_type>::value>>
		void push_back(value_type &&value)
		{
			VX_ASSERT(m_size < m_capacity);
			AllocatorBase::construct(m_pValues + m_size, std::forward<value_type>(value));
			++m_size;
		}

		void pop_back()
		{
			VX_ASSERT(m_size > 0);
			vx::AllocatorBase::destroy(&back());
			--m_size;
		}

		template<typename... _Valty>
		void emplace_back(_Valty&&... _Val)
		{
			VX_ASSERT(m_size < m_capacity);
			AllocatorBase::construct(m_pValues + m_size, std::forward<_Valty>(_Val)...);
			++m_size;
		}

		void clear()
		{
			AllocatorBase::rangeDestroy(m_pValues, m_pValues + m_size);
			m_size = 0;
		}

		reference front()
		{
			return *m_pValues;
		}

		const_reference front() const
		{
			return *m_pValues;
		}

		reference back()
		{
			return *(m_pValues + m_size - 1);
		}

		const_reference back() const
		{
			return *(m_pValues + m_size - 1);
		}

		reference operator[](size_type i)
		{
			return m_pValues[i];
		}

		const_reference operator[](size_type i) const
		{
			return m_pValues[i];
		}

		const_iterator begin() const
		{
			return const_iterator(m_pValues, this);
		}

		iterator begin()
		{
			return iterator(m_pValues, this);
		}

		const_iterator end() const
		{
			return const_iterator(m_pValues + m_size, this);
		}

		pointer data()
		{
			return m_pValues;
		}

		const_pointer data() const
		{
			return m_pValues;
		}

		iterator end()
		{
			return iterator(m_pValues + m_size, this);
		}

		size_type size() const
		{
			return m_size;
		}
	};
}

#endif