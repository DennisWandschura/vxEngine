#include <vxEngineLib/TaskManager.h>
#include <atomic>
#include <algorithm>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>
#include <vxEngineLib/ThreadSafeStack.h>
#include <vxEngineLib/atomic_float.h>

namespace vx
{
	struct TaskBuffer
	{
		ThreadSafeStack<LightTask*> m_tasks;
		atomic_float m_time;

		TaskBuffer()
			:m_tasks(),
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
		}

		bool pushTaskTS(LightTask* task, u32 capacity, f32 maxTime)
		{
			auto taskTime = task->getTimeMs();

			auto oldTime = m_time.load();

			f32 newTime;
			do
			{
				newTime = oldTime + taskTime;
				if (newTime >= maxTime)
					return false;

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
		}

		void clear()
		{
			m_tasks.clear();
			m_time.store(0.0f);
		}
	};

	class VX_ALIGN(64) LocalQueue
	{
		std::atomic<TaskBuffer*> m_front;
		TaskBuffer* m_back;
		f32 m_maxTime;
		u32 m_capacity;
		std::atomic_int m_counter;
		TaskBuffer m_buffers[2];

		std::atomic_int m_running;
		TaskManager* m_scheduler;

		void rescheduleTask(LightTask* task)
		{
			if (!pushTask(task))
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
			m_counter(),
			m_running({ 0 }),
			m_scheduler(nullptr)
		{
			m_counter.store(0);
			m_front.store(&m_buffers[0]);
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

		bool pushTask(LightTask* task)
		{
			m_counter.fetch_add(1);

			auto queue = m_front.load();

			bool result = queue->pushTaskTS(task, m_capacity, m_maxTime);

			m_counter.fetch_sub(1);

			return result;
		}

		void swapBuffer()
		{
			while (m_counter.load() != 0)
				;

			m_back = m_front.exchange(m_back);
		}

		void processBack()
		{
			auto backSize = m_back->sizeTS();
			if (backSize == 0)
			{
				std::this_thread::yield();
			}
			else
			{
				LightTask* waitingTask = nullptr;

				auto &backTasks = m_back->m_tasks;
				for (u32 i = 0; i < backSize; ++i)
				{
					auto task = backTasks[i];
					auto result = task->run();
					if (result == TaskReturnType::Retry)
					{
						rescheduleTask(task);
					}
					else if (result == TaskReturnType::WaitingForEvents)
					{
						if (waitingTask == nullptr)
						{
							waitingTask = task;
						}
						else
						{
							rescheduleTask(task);
						}
					}
				}

				if (waitingTask)
				{
					auto result = waitingTask->run();
					if (result != TaskReturnType::Success)
					{
						rescheduleTask(waitingTask);
					}
				}

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
		//printf("Worker Thread tid: %u\n", t_id);

		SmallObjAllocator allocator(1024);
		Task::setAllocator(&allocator);
		Event::setAllocator(&allocator);

		while (queue->isRunning())
		{
			queue->swapBuffer();
			queue->processBack();
		}
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

	void TaskManager::initialize(u32 count, u32 localQueueCapacity, f32 maxTimeMs, vx::StackAllocator* allocator)
	{
		if (count != 0)
		{
			m_queues = (LocalQueue**)allocator->allocate(sizeof(LocalQueue*) * count, 8);
			for (u32 i = 0; i < count; ++i)
			{
				auto ptr = (LocalQueue*)allocator->allocate(sizeof(LocalQueue), __alignof(LocalQueue));
				new (ptr) LocalQueue{};

				m_queues[i] = ptr;
			}

			m_queue = m_queues[0];
			m_queue->initialize(localQueueCapacity, maxTimeMs - 5.0f, this, allocator);

			auto threadCount = count - 1;
			if (threadCount != 0)
			{
				m_threads = (std::thread*)allocator->allocate(sizeof(std::thread) * threadCount, __alignof(std::thread));
				for (u32 i = 1; i < count; ++i)
				{
					auto ptr = m_queues[i];
					ptr->initialize(localQueueCapacity, maxTimeMs, this, allocator);

					new (&m_threads[i - 1]) std::thread(localThread, ptr, i);
				}
			}

			m_capacity = localQueueCapacity;
		}

		m_threadCount = count;
	}

	void TaskManager::initializeThread(std::atomic_uint* running)
	{
		m_running = running;

		m_allocator = std::make_unique<SmallObjAllocator>(1024);
		s_tid = 0;
		Task::setAllocator(m_allocator.get());
		Event::setAllocator(m_allocator.get());
	}

	void TaskManager::shutdown()
	{
		if (m_threadCount != 0)
		{
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

			if (m_queues[tid]->pushTask(task))
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
			m_queue->processBack();
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
						if (it->pushTask(task))
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

		for (u32 i = 0; i < m_threadCount - 1; ++i)
		{
			auto &it = m_threads[i];

			if (it.joinable())
				it.join();
		}
	}
}