#include <vxEngineLib/TaskManager.h>
#include <atomic>
#include <algorithm>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>

namespace vx
{
	struct TaskBuffer
	{
		LightTask** m_tasks;
		u32 m_size;
		f32 m_time;

		TaskBuffer()
			:m_tasks(nullptr),
			m_size(0),
			m_time(0.0f)
		{

		}

		~TaskBuffer()
		{
		}

		void initialize(u32 capacity, vx::StackAllocator* allocator)
		{
			m_tasks = (LightTask**)allocator->allocate(sizeof(LightTask*) * capacity, 8);
		}

		void swap(TaskBuffer &other)
		{
			std::swap(m_tasks, other.m_tasks);
			std::swap(m_size, other.m_size);
		}

		bool pushTask(LightTask* task, u32 capacity, f32 maxTime)
		{
			if (capacity <= m_size)
				return false;

			auto taskTime = task->getTimeMs();
			auto bufferTime = m_time + taskTime;

			if (bufferTime >= maxTime)
				return false;

			m_time = bufferTime;
			m_tasks[m_size++] = task;

			return true;
		}
	};

	class LocalQueue
	{
		struct VX_ALIGN(64)
		{
			TaskBuffer m_front;
			std::mutex m_frontMutex;
			f32 m_maxTime;
			u32 m_capacity;
		};

		TaskBuffer m_back;
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
			m_frontMutex(),
			m_back(),
			m_capacity(0),
			m_running({ 0 }),
			m_scheduler(nullptr)
		{

		}

		~LocalQueue()
		{

		}

		void initialize(u32 capacity, f32 maxTime, TaskManager* scheduler, vx::StackAllocator* allocator)
		{
			m_front.initialize(capacity, allocator);
			m_back.initialize(capacity, allocator);

			m_maxTime = maxTime;
			m_capacity = capacity;
			m_scheduler = scheduler;
			m_running.store(1);
		}

		bool pushTask(LightTask* task)
		{
			std::unique_lock<std::mutex> lock(m_frontMutex);
			return m_front.pushTask(task, m_capacity, m_maxTime);
		}

		void swapBuffer()
		{
			std::unique_lock<std::mutex> lock(m_frontMutex);
			m_front.swap(m_back);
		}

		void processBack()
		{
			if (m_back.m_size == 0)
			{
				std::this_thread::yield();
			}
			else
			{
				LightTask* waitingTask = nullptr;

				for (u32 i = 0; i < m_back.m_size; ++i)
				{
					auto task = m_back.m_tasks[i];
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

				m_back.m_size = 0;
				m_back.m_time = 0.0f;
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
		m_queues(),
		m_tasksBack(),
		m_capacity(0),
		m_threads(),
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
		count = std::max(1u, count);
		for (u32 i = 0; i < count; ++i)
		{
			auto ptr = (LocalQueue*)allocator->allocate(sizeof(LocalQueue), __alignof(LocalQueue));
			new (ptr) LocalQueue{};

		//	ptr->initialize(localQueueCapacity, maxTimeMs, this, allocator);
			m_queues.push_back(ptr);
		}

		m_queue = m_queues[0];
		m_queue->initialize(localQueueCapacity, maxTimeMs - 5.0f, this, allocator);
		for (u32 i = 1; i < count; ++i)
		{
			auto ptr = m_queues[i];
			ptr->initialize(localQueueCapacity, maxTimeMs, this, allocator);

			m_threads.push_back(std::thread(localThread, ptr, i));
		}

		m_capacity = localQueueCapacity;
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
		for (auto &it : m_queues)
		{
			it->~LocalQueue();
		}
		m_queues.clear();
		m_threads.clear();
	}

	void TaskManager::pushTask(LightTask* task)
	{
		auto tid = s_tid;
		pushTask(tid, task);
	}

	void TaskManager::pushTask(u32 tid, LightTask* task)
	{
		VX_ASSERT(tid >= 0);

		auto sz = m_queues.size();
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

			u32 avgSizePerQueue = static_cast<u32>((m_tasksBack.size() + m_queues.size() - 1) / m_queues.size());
			avgSizePerQueue = std::min(m_capacity, avgSizePerQueue);

			/*std::sort(m_tasksBack.begin(), m_tasksBack.end(), [](const Task* l, const Task* r)
			{
				return l->getPriority() > r->getPriority();
			});*/

			while (!m_tasksBack.empty())
			{
				for (auto &it : m_queues)
				{
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

		for (auto &it : m_queues)
		{
			it->signalStop();
		}

		for (auto &it : m_threads)
		{
			if (it.joinable())
				it.join();
		}
	}
}