#pragma once

#include <vector>
#include <functional>
#include <algorithm>
#include "Array.h"

namespace vx
{
	template<typename T, typename U, typename Cmp = std::less<T>>
	class Heap
	{
		typedef std::pair<T, U> value_type;
		typedef value_type& reference;
		typedef const reference const_reference;

		std::vector<value_type> m_container;

		struct Compare
		{
			bool operator()(const value_type &l, const value_type &r)
			{
				return Cmp()(l.first, r.first);
			}
		};

	public:
		Heap() = default;
		Heap(const Heap&) = delete;

		void push(const value_type &v)
		{
			m_container.push_back(v);
			std::push_heap(m_container.begin(), m_container.end(), Compare());
		}

		void push(value_type &&v)
		{
			m_container.push_back(std::move(v));
			std::push_heap(m_container.begin(), m_container.end(), Compare());
		}

		void pop()
		{
			std::pop_heap(m_container.begin(), m_container.end(), Compare());
			m_container.pop_back();
		}

		reference top()
		{
			return m_container.front();
		}

		const_reference top() const
		{
			return m_container.front();
		}

		void reserve(U32 n)
		{
			m_container.reserve(n);
		}
	};

	template<typename T, typename U, typename Cmp = std::less<T>>
	class HeapArray
	{

		typedef std::pair<T, U> value_type;
		typedef value_type& reference;
		typedef const reference const_reference;

		typedef vx::Array<value_type> MyContainer;
		typedef typename MyContainer::size_type size_type;
		typedef typename MyContainer::MyAllocator MyAllocator;

		struct Compare
		{
			bool operator()(const value_type &l, const value_type &r)
			{
				return Cmp()(l.first, r.first);
			}
		};

		MyContainer m_container;

	public:
		HeapArray() = default;
		HeapArray(const HeapArray&) = delete;

		HeapArray(size_type capacity, MyAllocator* pAllocator)
			:m_container(capacity, pAllocator)
		{
		}

		void push(const value_type &v)
		{
			m_container.push_back(v);
			std::push_heap(m_container.begin(), m_container.end(), Compare());
		}

		void push(value_type &&v)
		{
			m_container.push_back(std::move(v));
			std::push_heap(m_container.begin(), m_container.end(), Compare());
		}

		void pop()
		{
			std::pop_heap(m_container.begin(), m_container.end(), Compare());
			m_container.pop_back();
		}

		reference top()
		{
			return m_container.front();
		}

		const_reference top() const
		{
			return m_container.front();
		}
	};
}