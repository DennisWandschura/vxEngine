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

#include <vxEngineLib/TaskManager.h>
#include <atomic>
#include <algorithm>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>
#include <vxEngineLib/ThreadSafeStack.h>
#include <vxEngineLib/atomic_float.h>
#include <vxEngineLib/Logfile.h>
#include <vxLib/string.h>

namespace TaskManagerCpp
{
	/*
		Success,
	Failure,
	Retry,
	WaitingForEvents,
	Timeout
	*/
	const char* g_taskReturnTypeString[] =
	{
		"Success",
		"Failure",
		"Retry",
		"WaitingForEvents",
		"Timeout"
	};

	const u32 g_taskReturnTypeStringSize[] =
	{
		7,
		7,
		5,
		16,
		7
	};
}

namespace vx
{
	struct TaskBuffer
	{
		ThreadSafeStack<LightTask*> m_tasks;
		//LightTask** m_tasks;
		//mutable std::mutex m_mutex;
		//u32 m_size;
		//u32 m_capacity;
		atomic_float m_time;

		TaskBuffer()
			:m_tasks(),
			//m_mutex(),
			//m_size(0),
			//m_capacity(0),
			m_time()
		{
			m_time.store(0.0f);
		}

		~TaskBuffer()
		{
		}

		void initialize(u32 capacity, vx::StackAllocator* allocator)
		{
			auto ptr = allocator->allocate(sizeof(LightTask*) * capacity, 8);
			m_tasks.initialize(ptr, capacity);
			//m_tasks = (LightTask**)ptr;
			//vx::setZero(m_tasks, sizeof(LightTask*) * capacity);

			//m_capacity = capacity;
		}

		bool pushTaskTS(LightTask* task, u32 capacity, f32 maxTime)
		{
			auto taskTime = task->getTimeMs();

			/*std::lock_guard<std::mutex> lck(m_mutex);
			auto oldTime = m_time;
			auto newTime = oldTime + taskTime;

			if (newTime >= maxTime)
				return false;

			if (m_size >= m_capacity)
			{
				return false;
			}

			m_tasks[m_size++] = task;
			m_time = newTime;

			return true;*/

			auto oldTime = m_time.load();

			f32 newTime;
			do
			{
				newTime = oldTime + taskTime;
				if (newTime >= maxTime)
				{
					return false;
				}

			} while (!m_time.compare_exchange_weak(&oldTime, newTime));

			bool result = true;
			if (!m_tasks.pushTS(task))
			{
				auto oldTime = m_time.fetch_sub(taskTime);
				result = false;
			}

			return result;
		}

		u32 sizeTS() const
		{
			return m_tasks.sizeTS();
			//std::lock_guard<std::mutex> lck(m_mutex);
			//return m_size;
		}

		void clear()
		{
			/*for (u32 i = 0; i < m_size; ++i)
			{
				//delete(m_tasks[i]);
				m_tasks[i] = nullptr;
			}
			m_size = 0;
			m_time = 0.0f;*/

			m_tasks.clear();
			m_time.store(0.0f);
		}
	};

	class VX_ALIGN(64) LocalQueue
	{
		TaskBuffer* m_front;
		TaskBuffer* m_back;
		std::mutex m_mutex;
		f32 m_maxTime;
		u32 m_capacity;
		TaskBuffer m_buffers[2];

		std::atomic_int m_running;
		TaskManager* m_scheduler;

		void rescheduleTask(LightTask* task, Logfile* logFile)
		{
			if (!pushTask(task, logFile))
			{
				m_scheduler->pushTask(task);
			}
		}

	public:
		LocalQueue()
			:m_front(),
			m_back(nullptr),
			m_capacity(0),
			m_buffers(),
			m_running({ 0 }),
			m_scheduler(nullptr)
		{
			m_front = &m_buffers[0];
			m_back = &m_buffers[1];
		}

		~LocalQueue()
		{

		}

		void initialize(u32 capacity, f32 maxTime, TaskManager* scheduler, vx::StackAllocator* allocator)
		{
			m_buffers[0].initialize(capacity, allocator);
			m_buffers[1].initialize(capacity, allocator);

			m_maxTime = maxTime;
			m_capacity = capacity;
			m_scheduler = scheduler;
			m_running.store(1);
		}

		bool pushTask(LightTask* task, Logfile* logFile)
		{
			auto appendTaskToLogfile = [](Logfile* logFile, LightTask* task)
			{
				u32 strSize = 0;
				auto taskName = task->getName(&strSize);

				char buffer[24];
				s32 size = snprintf(buffer, sizeof(buffer), " %p add\n", task);
				VX_ASSERT(size >= 0);

				logFile->append(taskName, strSize - 1);
				logFile->append(buffer, size);
			};

			std::lock_guard<std::mutex> lck(m_mutex);

			//auto queue = m_front.load();
			auto queue = m_front;

			bool result = queue->pushTaskTS(task, m_capacity, m_maxTime);
			if (result)
			{
				appendTaskToLogfile(logFile, task);
			}

			return result;
		}

		void swapBuffer()
		{
			std::lock_guard<std::mutex> lck(m_mutex);
			std::swap(m_back, m_front);
		}

		void processBack(Logfile* logFile)
		{
			auto appendTaskResultToLogfile = [](Logfile* logFile, LightTask* task, TaskReturnType result)
			{
				u32 strSize = 0;
				auto taskName = task->getName(&strSize);

				u32 resultIndex = (u32)result;
				auto resultString = TaskManagerCpp::g_taskReturnTypeString[resultIndex];
				u32 resultSize = TaskManagerCpp::g_taskReturnTypeStringSize[resultIndex];

				char buffer[24];
				s32 size = snprintf(buffer, sizeof(buffer), " %p: ", task);
				VX_ASSERT(size >= 0);

				logFile->append(taskName, strSize - 1);
				logFile->append(buffer, size);
				logFile->append(resultString, resultSize);
				logFile->append('\n');
			};

			auto backSize = m_back->sizeTS();
			if (backSize == 0)
			{
				std::this_thread::yield();
			}
			else
			{
				//
				//LightTask* waitingTask = nullptr;

				//const char str[] = "processing tasks\n";
				//logFile->append(str, 18);

				auto &backTasks = m_back->m_tasks;
				for (u32 i = 0; i < backSize; ++i)
				{
					auto task = backTasks[i];
					//printf("running task %p\n", task);
					auto result = task->run();

					appendTaskResultToLogfile(logFile, task, result);

					if (result == TaskReturnType::Retry)
					{
						rescheduleTask(task, logFile);
					}
					else if (result == TaskReturnType::WaitingForEvents)
					{
						/*if (waitingTask == nullptr)
						{
							waitingTask = task;
						}*/
						//else
						{
							rescheduleTask(task, logFile);
						}
					}
					else
					{
						//printf("%p\n", task);
						delete(task);
					}
				}

				/*if (waitingTask)
				{
					auto result = waitingTask->run();

					appendTaskResultToLogfile(logFile, waitingTask, result);

					if (result != TaskReturnType::Success)
					{
						rescheduleTask(waitingTask, logFile);
					}
				}*/

				m_back->clear();
			}
		}

		bool isRunning() const
		{
			return (m_running.load() != 0);
		}

		void signalStop()
		{
			m_running.store(0);
		}
	};

	thread_local u32 s_tid{ 0 };

	void localThread(LocalQueue* queue, u32 tid)
	{
		s_tid = tid;

		auto t_id = std::this_thread::get_id();

		char buffer[24];
		vx::setZero(buffer, sizeof(buffer));
		snprintf(buffer, sizeof(buffer), "tasklog-tid-%u.txt", tid);

		Logfile taskLog;
		taskLog.create(buffer);

		SmallObjAllocator allocator(1024);
		Task::setAllocator(&allocator);
		Event::setAllocator(&allocator);

		while (queue->isRunning())
		{
			queue->swapBuffer();
			queue->processBack(&taskLog);
		}

		taskLog.close();
	}

	TaskManager::TaskManager()
		:m_tasksFront(),
		m_mutexFront(),
		m_queues(nullptr),
		m_tasksBack(),
		m_capacity(0),
		m_threadCount(0),
		m_threads(nullptr),
		m_running(nullptr),
		m_allocator()
	{
	}

	TaskManager::~TaskManager()
	{
		shutdown();
	}

	void TaskManager::initialize(u32 threadCount, u32 localQueueCapacity, f32 maxTimeMs, vx::StackAllocator* allocator)
	{
		if (threadCount != 0)
		{
			m_queues = (LocalQueue**)allocator->allocate(sizeof(LocalQueue*) * threadCount, 8);
			for (u32 i = 0; i < threadCount; ++i)
			{
				auto ptr = (LocalQueue*)allocator->allocate(sizeof(LocalQueue), __alignof(LocalQueue));
				new (ptr) LocalQueue{};

				m_queues[i] = ptr;
			}

			m_queue = m_queues[0];
			m_queue->initialize(localQueueCapacity, maxTimeMs - 5.0f, this, allocator);
			m_taskLog.create("tasklog-tid-0.txt");

			auto workerThreadCount = threadCount - 1;
			if (workerThreadCount != 0)
			{
				m_threads = (std::thread*)allocator->allocate(sizeof(std::thread) * workerThreadCount, __alignof(std::thread));
				for (u32 i = 1; i < threadCount; ++i)
				{
					auto ptr = m_queues[i];
					ptr->initialize(localQueueCapacity, maxTimeMs, this, allocator);

					new (&m_threads[i - 1]) std::thread(localThread, ptr, i);
				}
			}

			m_capacity = localQueueCapacity;
		}

		m_threadCount = threadCount;
	}

	void TaskManager::initializeThread(std::atomic_uint* running)
	{
		m_running = running;

		m_allocator = vx::aligned_ptr<SmallObjAllocator>(1024);
		s_tid = 0;
		Task::setAllocator(m_allocator.get());
		Event::setAllocator(m_allocator.get());
	}

	void TaskManager::shutdown()
	{
		if (m_threadCount != 0)
		{
			m_taskLog.close();
			for (u32 i = 0; i < m_threadCount; ++i)
			{
				m_queues[i]->~LocalQueue();
			}
			m_queues = nullptr;

			for (u32 i = 0; i < (m_threadCount - 1); ++i)
			{
				auto &it = m_threads[i];
				if (it.joinable())
					it.join();

				it.~thread();
			}
			m_threads = nullptr;

			m_threadCount = 0;
		}
	}

	void TaskManager::pushTask(LightTask* task)
	{
		auto tid = s_tid;
		pushTask(tid, task);
	}

	void TaskManager::pushTask(u32 tid, LightTask* task)
	{
		VX_ASSERT(tid >= 0);

		auto sz = m_threadCount;
		VX_ASSERT(tid < sz);
			
		if (sz == 0)
		{
			task->run();
		}
		else
		{
			bool inserted = false;

			if (m_queues[tid]->pushTask(task, &m_taskLog))
			{
				inserted = true;
			}

			if (!inserted)
			{
				std::unique_lock<std::mutex> lock(m_mutexFront);
				m_tasksFront.push_back(task);
			}
		}
	}

	void TaskManager::swapBuffer()
	{
		std::unique_lock<std::mutex> lock(m_mutexFront);
		m_tasksFront.swap(m_tasksBack);
	}

	void TaskManager::doWork()
	{
		if (m_queue->isRunning())
		{
			m_queue->swapBuffer();
			m_queue->processBack(&m_taskLog);
		}
	}

	void TaskManager::update()
	{
		if (!m_tasksBack.empty())
		{
			u32 distributedTasks = 0;

			auto threadCount = m_threadCount;

			u32 avgSizePerQueue = static_cast<u32>((m_tasksBack.size() + threadCount - 1) / threadCount);
			avgSizePerQueue = std::min(m_capacity, avgSizePerQueue);

			/*std::sort(m_tasksBack.begin(), m_tasksBack.end(), [](const Task* l, const Task* r)
			{
				return l->getPriority() > r->getPriority();
			});*/

			while (!m_tasksBack.empty())
			{
				for (u32 i = 0; i < threadCount; ++i)
				{
					auto &it = m_queues[i];
					if (m_tasksBack.empty())
						break;

					auto count = std::min(avgSizePerQueue, (u32)m_tasksBack.size());

					for (u32 i = 0; i < count; ++i)
					{
						auto task = m_tasksBack.back();
						if (it->pushTask(task, &m_taskLog))
						{
							m_tasksBack.pop_back();
							++distributedTasks;
						}
					}

				}
			}
		}

		doWork();
	}

	void TaskManager::stop()
	{
		if (m_running)
		{
			m_running->store(0);
		}

		for (u32 i = 0; i < m_threadCount; ++i)
		{
			m_queues[i]->signalStop();
		}

		if (m_threadCount > 0)
		{
			for (u32 i = 0; i < m_threadCount - 1; ++i)
			{
				auto &it = m_threads[i];

				if (it.joinable())
					it.join();
			}
		}
	}
}