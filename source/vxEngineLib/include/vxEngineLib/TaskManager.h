#pragma once

#include <vxEngineLib/Task.h>
#include <vector>
#include <mutex>

namespace vx
{
	class LocalQueue;

	class TaskManager
	{
		struct alignas(64)
		{
			std::vector<Task*> m_tasksFront;
			std::mutex m_mutexFront;
			std::vector<LocalQueue*> m_queues;
		};

		std::vector<Task*> m_tasksBack;
		int m_capacity;
		std::vector<std::thread> m_threads;

	public:
		TaskManager();

		~TaskManager();

		void initialize(int count, int capacity);

		void pushTask(Task* task);

		void swapBuffer();

		void update();

		void stop();
	};
}