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

#include <vector>
#include <functional>
#include <algorithm>
#include <vxLib/Container/array.h>

namespace vx
{
	template<typename T, typename U, typename Cmp = std::less<T>>
	class Heap
	{
		typedef std::pair<T, U> value_type;
		typedef value_type& reference;
		typedef const T& const_reference;

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

		void reserve(u32 n)
		{
			m_container.reserve(n);
		}
	};

	template<typename T, typename U, typename Cmp = std::less<T>>
	class HeapArray
	{
		typedef std::pair<T, U> value_type;
		typedef value_type& reference;
		typedef const value_type& const_reference;

		typedef vx::array<value_type> MyContainer;
		typedef typename MyContainer::size_type size_type;

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

		template<typename Alloc>
		HeapArray(size_type capacity, Alloc* pAllocator)
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