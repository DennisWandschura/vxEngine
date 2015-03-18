#include "thread.h"

namespace vx
{
	thread::thread()
		:m_thread()
	{

	}

	thread::thread(std::thread &&t)
		: m_thread(std::move(t))
	{

	}

	thread::thread(thread &&rhs)
		: m_thread(std::move(rhs.m_thread))
	{

	}

	thread::~thread()
	{
		join();
	}

	thread& thread::operator = (std::thread &&rhs)
	{
		m_thread = std::move(rhs);
		return *this;
	}

	thread& thread::operator = (thread &&rhs)
	{
		if (this != &rhs)
		{
			std::swap(m_thread, rhs.m_thread);
		}

		return *this;
	}

	U32 thread::hardware_concurrency() noexcept
	{
		return std::thread::hardware_concurrency();
	}

		void thread::detach() noexcept
	{
		if (m_thread.joinable())
		{
			m_thread.detach();
		}
	}

	void thread::join() noexcept
	{
		if (m_thread.joinable())
		{
			m_thread.join();
		}
	}

	std::thread::id thread::get_id() const noexcept
	{
		return m_thread.get_id();
	}
}