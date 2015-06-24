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

#include <Windows.h>
#include <vxEngineLib/mutex.h>

namespace vx
{
	class SRWMutex
	{
		SRWLOCK m_lock;

	public:
		SRWMutex() :m_lock(){ InitializeSRWLock(&m_lock); }

		void lockShared()
		{
			AcquireSRWLockShared(&m_lock);
		}

		bool tryLockShared()
		{
			return TryAcquireSRWLockShared(&m_lock) != 0;
		}

		void lock()
		{
			AcquireSRWLockExclusive(&m_lock);
		}

		bool tryLock()
		{
			return TryAcquireSRWLockExclusive(&m_lock) != 0;
		}

		void unlockShared()
		{
			ReleaseSRWLockShared(&m_lock);
		}

		void unlock()
		{
			ReleaseSRWLockExclusive(&m_lock);
		}
	};

	template<class _Mutex>
	class shared_lock_guard
	{	// class with destructor that unlocks mutex
	public:
		typedef _Mutex mutex_type;

		explicit shared_lock_guard(_Mutex& _Mtx)
			: _MyMutex(_Mtx)
		{	// construct and lock
			_MyMutex.lockShared();
		}

		~shared_lock_guard() _NOEXCEPT
		{	// unlock
			_MyMutex.unlockShared();
		}

		shared_lock_guard(const shared_lock_guard&) = delete;
		shared_lock_guard& operator=(const shared_lock_guard&) = delete;

	private:
		_Mutex& _MyMutex;
	};
}