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
#include <vxEngineLib/TaskManager.h>
#include <pxtask/PxTask.h>
#include <new>
#include <cstdio>
#include <vxEngineLib/CpuTimer.h>

class PhysxTask : public Task
{
	static thread_local f32 s_time;
	static thread_local u64 s_counter;

	physx::PxBaseTask* m_task;

public:
	explicit PhysxTask(physx::PxBaseTask* task)
		: m_task(task)
	{
	}

	PhysxTask(PhysxTask &&rhs)
		:Task(std::move(rhs)), m_task(rhs.m_task)
	{
	}

	~PhysxTask()
	{

	}

	TaskReturnType run() override
	{
		CpuTimer timer;

		m_task->run();
		m_task->release();

		auto time = timer.getTimeMs();
		auto oldCounter = s_counter++;
		s_time = (s_time * oldCounter + time) / s_counter;

		return TaskReturnType::Success;
	}

	f32 getTimeMs() const override
	{
		return s_time;
	}
};

thread_local f32 PhysxTask::s_time{ 0.0f };
thread_local u64 PhysxTask::s_counter{ 0 };

PhysicsCpuDispatcher::PhysicsCpuDispatcher()
	:m_taskManager(nullptr)
{
}

PhysicsCpuDispatcher::~PhysicsCpuDispatcher()
{
}

void PhysicsCpuDispatcher::initialize(vx::TaskManager* taskManager)
{
	m_taskManager = taskManager;
}

void PhysicsCpuDispatcher::submitTask(physx::PxBaseTask& task)
{
	auto newTask = new PhysxTask(&task);

	m_taskManager->pushTask(newTask, false);
}

physx::PxU32 PhysicsCpuDispatcher::getWorkerCount() const
{
	return 1;
}