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

#include "PhysicsCpuDispatcher.h"
#include "TaskManager.h"
#include <pxtask/PxTask.h>
#include <new>
#include <cstdio>

class PhysxTask : public Task
{
	physx::PxBaseTask* m_task;

public:
	PhysxTask(physx::PxBaseTask* task)
		:m_task(task)
	{
	}

	PhysxTask(PhysxTask &&rhs)
		:m_task(rhs.m_task)
	{
	}

	~PhysxTask()
	{

	}

	TaskReturnType run() override
	{
		m_task->run();
		m_task->release();

		return TaskReturnType::Success;
	}

	Task* move(vx::Allocator* allocator) override
	{
		auto ptr = (PhysxTask*)allocator->allocate(sizeof(PhysxTask), __alignof(PhysxTask));

		new (ptr) PhysxTask(std::move(*this));

		return ptr;
	}
};

PhysicsCpuDispatcher::PhysicsCpuDispatcher()
	:m_taskManager(nullptr)
{

}

PhysicsCpuDispatcher::~PhysicsCpuDispatcher()
{

}

void PhysicsCpuDispatcher::initialize(TaskManager* taskManager)
{
	m_taskManager = taskManager;
}

void PhysicsCpuDispatcher::submitTask(physx::PxBaseTask& task)
{
	m_taskManager->queueTask<PhysxTask>(&task);
}

physx::PxU32 PhysicsCpuDispatcher::getWorkerCount() const
{
	return 1;
}