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

#include "TaskManager.h"
#include <algorithm>
#include "AllocationManager.h"

TaskManager::TaskManager()
	:m_tasks(nullptr),
	m_backTasks(nullptr),
	m_size(0),
	m_backSize(0),
	m_capacity(0),
	m_allocator(),
	m_backAllocator()
{

}

TaskManager::~TaskManager()
{

}

void TaskManager::initialize(vx::StackAllocator* allocator, u32 maxTaskCount, AllocationManager* allocManager)
{
	m_tasks = (Task**)allocator->allocate(sizeof(Task*) * maxTaskCount, __alignof(Task*));
	VX_ASSERT(m_tasks != nullptr);
	m_backTasks = (Task**)allocator->allocate(sizeof(Task*) * maxTaskCount, __alignof(Task*));
	VX_ASSERT(m_backTasks != nullptr);

	const auto memorySize = 64 KBYTE;
	auto memory = allocator->allocate(memorySize, 64);
	VX_ASSERT(memory != nullptr);
	m_allocator = vx::StackAllocator(memory, memorySize);

	memory = allocator->allocate(memorySize, 64);
	VX_ASSERT(memory != nullptr);
	m_backAllocator = vx::StackAllocator(memory, memorySize);

	m_capacity = maxTaskCount;

	allocManager->registerAllocator(&m_allocator, "taskAlloc0");
	allocManager->registerAllocator(&m_backAllocator, "taskAlloc1");
}

void TaskManager::shutdown()
{
	for (u32 i = 0; i < m_size; ++i)
	{
		m_tasks[i]->~Task();
	}

	for (u32 i = 0; i < m_backSize; ++i)
	{
		m_backTasks[i]->~Task();
	}

	m_allocator.clear();
	m_backAllocator.clear();
}

void TaskManager::update()
{
	m_allocator.swap(m_backAllocator);
	std::swap(m_size, m_backSize);
	std::swap(m_tasks, m_backTasks);

	auto tasks = m_backTasks;
	auto size = m_backSize;

	for (u32 i = 0; i < size; ++i)
	{
		auto task = tasks[i];

		task->run();

		/*if (!task->isFinished())
		{
			auto newTask = task->move(&m_allocator);
			queueTask(newTask);
		}*/
	}

	m_backAllocator.clear();
	m_backSize = 0;
}

void TaskManager::queueTask(Task* task)
{
	VX_ASSERT(m_size < m_capacity);

	m_tasks[m_size++] = task;
}