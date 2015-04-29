#pragma once

#include <vxLib\types.h>
#include <thread>

namespace vx
{
	class thread
	{
		std::thread m_thread;

	public:
		thread();
		
		template<class _Fn, class... _Args>
		explicit thread(_Fn&& _Fx, _Args&&... _Ax)
			:m_thread(std::forward<_Fn>(_Fx), std::forward<_Args>(_Ax)...)
		{

		}

		thread(std::thread &&t);

		thread(const thread&) = delete;
		thread(thread &&rhs);

		~thread();

		thread& operator=(std::thread &&rhs);
		thread& operator=(const thread&) = delete;
		thread& operator=(thread &&rhs);

		static U32 hardware_concurrency() noexcept;

		bool joinable() const noexcept;
		void detach() noexcept;
		void join() noexcept;

		std::thread::id get_id() const noexcept;
	};
}