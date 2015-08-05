#include <vxEngineLib/TaskManager.h>
#include <atomic>
#include <algorithm>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/SmallObjAllocator.h>

namespace vx
{
	struct Buffer
	{
		Task** m_tasks;
		u32 m_size;
		f32 m_time;

		Buffer()
			:m_tasks(nullptr),
			m_size(0),
			m_time(0.0f)
		{

		}

		~Buffer()
		{
		}

		void initialize(u32 capacity, vx::StackAllocator* allocator)
		{
			m_tasks = (Task**)allocator->allocate(sizeof(Task*) * capacity, 8);
		}

		void swap(Buffer &other)
		{
			std::swap(m_tasks, other.m_tasks);
			std::swap(m_size, other.m_size);
		}

		bool pushTask(Task* task, u32 capacity, f32 maxTime, bool ignoreTime)
		{
			if (capacity <= m_size)
				return false;

			auto taskTime = task->getTimeMs();
			auto bufferTime = m_time + taskTime;

			if (!ignoreTime)
			{
				if (bufferTime >= maxTime)
					return false;
			}

			m_time = bufferTime;
			m_tasks[m_size++] = task;
			return true;
		}
	};

	class LocalQueue
	{
		struct VX_ALIGN(64)
		{
			Buffer m_front;
			std::mutex m_frontMutex;
			f32 m_maxTime;
			u32 m_capacity;
		};

		Buffer m_back;
		std::atomic_int m_running;
		TaskManager* m_scheduler;

		void rescheduleTask(Task* task)
		{
			if (!pushTask(task, true))
			{
				m_scheduler->pushTask(task, true);
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

		bool pushTask(Task* task, bool ignoreTime)
		{
			std::unique_lock<std::mutex> lock(m_frontMutex);
			return m_front.pushTask(task, m_capacity, m_maxTime, ignoreTime);
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
				Task* waitingTask = nullptr;

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
		m_running(nullptr)
	{

	}

	TaskManager::~TaskManager()
	{
		shutdown();
	}

	void TaskManager::initialize(u32 count, u32 capacity, f32 maxTime, vx::StackAllocator* allocator)
	{
		for (u32 i = 0; i < count; ++i)
		{
			auto ptr = (LocalQueue*)allocator->allocate(sizeof(LocalQueue), __alignof(LocalQueue));
			new (ptr) LocalQueue{};

			ptr->initialize(capacity, maxTime, this, allocator);
			m_queues.push_back(ptr);

			m_threads.push_back(std::thread(localThread, ptr, i));
		}

		m_capacity = capacity;
	}

	void TaskManager::initializeThread(std::atomic_uint* running)
	{
		m_running = running;
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

	void TaskManager::pushTask(Task* task, bool ignoreTime)
	{
		auto tid = s_tid;
		pushTask(tid, task, ignoreTime);
	}

	void TaskManager::pushTask(u32 tid, Task* task, bool ignoreTime)
	{
		bool inserted = false;

		if (m_queues[tid]->pushTask(task, ignoreTime))
		{
			inserted = true;
		}

		if (!inserted)
		{
			std::unique_lock<std::mutex> lock(m_mutexFront);
			m_tasksFront.push_back(task);
		}
	}

	void TaskManager::swapBuffer()
	{
		std::unique_lock<std::mutex> lock(m_mutexFront);
		m_tasksFront.swap(m_tasksBack);
	}

	void TaskManager::update()
	{
		if (m_tasksBack.empty())
		{
			std::this_thread::yield();
		}
		else
		{
			u32 distributedTasks = 0;

			u32 avgSizePerQueue = (m_tasksBack.size() + m_queues.size() - 1) / m_queues.size();
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
						if (it->pushTask(task, false))
						{
							m_tasksBack.pop_back();
							++distributedTasks;
						}
					}

				}
			}
		}
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