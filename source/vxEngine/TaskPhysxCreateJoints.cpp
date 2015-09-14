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
#include "TaskPhysxCreateJoints.h"
#include <vxEngineLib/Scene.h>
#include "PhysicsAspect.h"
#include <vxEngineLib/Joint.h>
#include <vxEngineLib/CpuTimer.h>

thread_local f32 TaskPhysxCreateJoints::s_time{ 0.0f };
thread_local u64 TaskPhysxCreateJoints::s_counter{ 0 };

TaskPhysxCreateJoints::TaskPhysxCreateJoints(const Scene* scene, PhysicsAspect* physicsAspect, std::vector<Event> &&events, const Event &blockEvt)
	:Task(blockEvt, std::move(events)),
	m_scene(scene),
	m_physicsAspect(physicsAspect)
{

}

TaskPhysxCreateJoints::~TaskPhysxCreateJoints()
{

}

TaskReturnType TaskPhysxCreateJoints::runImpl()
{
	auto status = m_events[0].getStatus();
	while (status != EventStatus::Complete)
	{
		status = m_events[0].getStatus();
	}

	CpuTimer timer;

	auto joints = m_scene->getJoints();
	auto jointCount = m_scene->getJointCount();

	auto result = TaskReturnType::Success;

	for (u32 i = 0; i < jointCount; ++i)
	{
		auto &it = joints[i];

		if (m_physicsAspect->createJoint(it) == nullptr)
		{
			result = TaskReturnType::Retry;
		}
	}

	auto time = timer.getTimeMiliseconds();
	auto oldCounter = s_counter++;

	s_time = (s_time * oldCounter + time) / s_counter;

	return result;
}

f32 TaskPhysxCreateJoints::getTimeMs() const
{
	return s_time;
}