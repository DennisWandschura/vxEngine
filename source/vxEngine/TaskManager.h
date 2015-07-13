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

class AllocationManager;

#include <vxLib/Allocator/StackAllocator.h>
#include "Task.h"

class TaskManager
{
	Task** m_tasks;
	Task** m_backTasks;
	u32 m_size;
	u32 m_backSize;
	u32 m_capacity;
	vx::StackAllocator m_allocator;
	vx::StackAllocator m_backAllocator;

	void queueTask(Task* task);

public:
	TaskManager();
	~TaskManager();

	void initialize(vx::StackAllocator* allocator, u32 maxTaskCount, AllocationManager* allocManager);
	void shutdown();

	void update();

	template<typename T, typename ...Args>
	void queueTask(Args&& ...args)
	{
		auto ptr = (T*)m_allocator.allocate(sizeof(T), __alignof(T));
		VX_ASSERT(ptr != nullptr);

		new (ptr) T(std::forward<Args>(args)...);

		queueTask(ptr);
	}
};