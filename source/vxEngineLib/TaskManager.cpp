#include <vxEngineLib/TaskManager.h>
#include <atomic>
#include <algorithm>

namespace vx
{
	struct Buffer
	{
		Task** m_tasks;
		int m_size;

		Buffer()
			:m_tasks(nullptr),
			m_size(0)
		{

		}

		~Buffer()
		{

		}

		void swap(Buffer &other)
		{
			std::swap(m_tasks, other.m_tasks);
			std::swap(m_size, other.m_size);
		}

		bool pushTask(Task* task, int capacity)
		{
			if (capacity <= m_size)
				return false;

			m_tasks[m_size++] = task;
			return true;
		}
	};

	class LocalQueue
	{
		struct alignas(64)
		{
			Buffer m_front;
			std::mutex m_frontMutex;
		};

		Buffer m_back;
		int m_capacity;
		std::atomic_int m_running;
		TaskManager* m_scheduler;

	public:
		LocalQueue()
			:m_front(),
			m_frontMutex(),
			m_back(),
			m_capacity(0),
			m_running(0),
			m_scheduler(nullptr)
		{

		}

		void initialize(int capacity, TaskManager* scheduler)
		{
			m_front.m_tasks = new Task*[capacity];
			m_back.m_tasks = new Task*[capacity];

			m_capacity = capacity;
			m_scheduler = scheduler;
			m_running.store(1);
		}

		bool pushTask(Task* task)
		{
			std::unique_lock<std::mutex> lock(m_frontMutex);
			return m_front.pushTask(task, m_capacity);
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
				for (int i = 0; i < m_back.m_size; ++i)
				{
					auto task = m_back.m_tasks[i];
					auto result = task->run();
					if (result == TaskReturnType::Retry)
					{
						if (!pushTask(task))
						{
							m_scheduler->pushTask(task);
						}
					}
				}

				m_back.m_size = 0;
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

	thread_local int s_tid{ 0 };

	void localThread(LocalQueue* queue, int tid)
	{
		s_tid = tid;

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
		m_threads()
	{

	}

	TaskManager::~TaskManager()
	{

	}

	void TaskManager::initialize(int count, int capacity)
	{
		for (int i = 0; i < count; ++i)
		{
			auto ptr = (LocalQueue*)_aligned_malloc(sizeof(LocalQueue), __alignof(LocalQueue));
			new (ptr) LocalQueue{};

			ptr->initialize(capacity, this);
			m_queues.push_back(ptr);

			m_threads.push_back(std::thread(localThread, ptr, i));
		}

		m_capacity = capacity;
	}

	void TaskManager::pushTask(Task* task)
	{
		auto tid = s_tid;
		bool inserted = false;

		if (tid < m_queues.size())
		{
			if (m_queues[tid]->pushTask(task))
			{
				inserted = true;
			}
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
			int distributedTasks = 0;

			int avgSizePerQueue = (m_tasksBack.size() + m_queues.size() - 1) / m_queues.size();
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

					auto count = std::min(avgSizePerQueue, (int)m_tasksBack.size());

					for (int i = 0; i < count; ++i)
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
	}

	void TaskManager::stop()
	{
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