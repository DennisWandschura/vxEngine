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

#include <atomic>
#include <thread>

namespace vx
{
	// LOCK PROPERTIES
	struct adopt_lock_t
	{	// indicates adopt lock
	};

	struct defer_lock_t
	{	// indicates defer lock
	};

	struct try_to_lock_t
	{	// indicates try to lock
	};

	const adopt_lock_t adopt_lock{};
	const defer_lock_t defer_lock{};
	const try_to_lock_t try_to_lock{};

	class mutex
	{
		std::atomic_flag m_flag;

	public:
		mutex() :m_flag() { m_flag.clear(); }
		mutex(const mutex&) = delete;
		mutex(mutex&&) = delete;

		mutex& operator=(const mutex&) = delete;
		mutex& operator=(mutex&&) = delete;

		void lock()
		{
			while (m_flag.test_and_set())
				std::this_thread::yield();
		}

		bool try_lock()
		{
			return !m_flag.test_and_set();
		}

		void unlock()
		{
			m_flag.clear();
		}
	};

	template<typename MUTEX>
	class unique_lock
	{
		MUTEX* m_mutex;
		bool m_owns;

	public:
		unique_lock() noexcept :m_mutex(nullptr), m_owns(false) {}

		unique_lock(const unique_lock&) = delete;
		unique_lock(unique_lock &&rhs)
			:m_mutex(rhs.m_mutex), m_owns(rhs.m_owns) 
		{
			rhs.m_mutex = nullptr;
			rhs.m_owns = false;
		}

		explicit unique_lock(MUTEX& m)
			:m_mutex(&m),
			m_owns(false)
		{
			m_mutex->lock();
			m_owns = true;
		}

		unique_lock(MUTEX& _Mtx, defer_lock_t) noexcept
			: m_mutex(&_Mtx), m_owns(false)
		{	// construct but don't lock
		}

		unique_lock(MUTEX& _Mtx, try_to_lock_t)
			: m_mutex(&_Mtx), m_owns(m_mutex->try_lock())
		{	// construct and try to lock
		}

		~unique_lock() noexcept
		{	// clean up
			if (m_owns)
			{
				m_mutex->unlock();
			}
		}

		unique_lock& operator=(const unique_lock&) = delete;
		unique_lock& operator=(unique_lock &&rhs)
		{
			if (this != &rhs)
			{
				swap(rhs);
			}
			return *this;
		}

		void lock()
		{
			VX_ASSERT(!m_owns);

			m_mutex->lock();
			m_owns = true;
		}

		bool try_lock()
		{	// try to lock the mutex
			VX_ASSERT(!m_owns);

			m_owns = m_mutex->try_lock();
			return (m_owns);
		}

		void unlock()
		{
			VX_ASSERT(m_owns);

			m_mutex->unlock();
			m_owns = false;
		}

		void swap(unique_lock &other)
		{
			std::swap(m_mutex, rhs.m_mutex);
			std::swap(m_owns, rhs.m_owns);
		}

		bool owns_lock() const noexcept
		{	// return true if this object owns the lock
			return (m_owns);
		}

		explicit operator bool() const noexcept
		{	// return true if this object owns the lock
			return (m_owns);
		}
	};

	template<typename MUTEX>
	class lock_guard
	{
		MUTEX& m_mutex;

	public:
		explicit lock_guard(MUTEX& m)
			:m_mutex(m)
		{
			m_mutex.lock();
		}

		lock_guard(const lock_guard&) = delete;
		lock_guard(lock_guard&&) = delete;

		~lock_guard()
		{
			m_mutex.unlock();
		}

		lock_guard& operator=(const lock_guard&) = delete;
		lock_guard& operator=(lock_guard&&) = delete;
	};
}