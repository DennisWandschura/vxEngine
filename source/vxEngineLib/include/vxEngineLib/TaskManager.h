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

#include <vxEngineLib/Task.h>
#include <vector>
#include <mutex>
#include <atomic>
#include <thread>

namespace vx
{
	class LocalQueue;
	class StackAllocator;

	class TaskManager
	{
		struct VX_ALIGN(64)
		{
			std::vector<LightTask*> m_tasksFront;
			std::mutex m_mutexFront;
			std::vector<LocalQueue*> m_queues;
		};

		std::vector<LightTask*> m_tasksBack;
		u32 m_capacity;
		std::vector<std::thread> m_threads;
		std::atomic_uint* m_running;
		LocalQueue* m_queue;
		std::unique_ptr<SmallObjAllocator> m_allocator;

		void doWork();

	public:
		TaskManager();

		~TaskManager();

		void initialize(u32 threadCount, u32 localQueueCapacity, f32 maxTimeMs, vx::StackAllocator* allocator);
		void initializeThread(std::atomic_uint* running);
		void shutdown();

		void pushTask(LightTask* task);
		void pushTask(u32 tid, LightTask* task);

		void swapBuffer();

		void update();

		void stop();
	};
}