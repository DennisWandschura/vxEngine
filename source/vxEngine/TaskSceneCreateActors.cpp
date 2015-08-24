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
#include "TaskSceneCreateActors.h"
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/RenderAspectInterface.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/CpuTimer.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/MessageTypes.h>
#include "PhysicsAspect.h"
#include <vxEngineLib/Locator.h>

thread_local f32 TaskSceneCreateActors::s_time{0.0f};
thread_local u64 TaskSceneCreateActors::s_counter{0};

TaskSceneCreateActors::TaskSceneCreateActors(const Event &evt, std::vector<Event> &&events, const Scene* scene, RenderAspectInterface* renderAspect, PhysicsAspect* physicsAspect)
	:Task(evt, std::move(events)),
	m_scene(scene),
	m_renderAspect(renderAspect),
	m_physicsAspect(physicsAspect)
{

}

TaskSceneCreateActors::~TaskSceneCreateActors()
{

}

TaskReturnType TaskSceneCreateActors::runImpl()
{
	CpuTimer timer;

	auto spawns = m_scene->getSpawns();
	auto spawnCount = m_scene->getSpawnCount();

	auto msgManager = Locator::getMessageManager();

	for (u32 i = 0; i < spawnCount; ++i)
	{
		auto &it = spawns[i];

		if (it.type != PlayerType::Human)
		{
			auto &actors = m_scene->getActors();
			auto itActor = actors.find(it.sid);

			vx::Transform transform;
			transform.m_qRotation = vx::float4(0, 0, 0, 1);
			transform.m_scaling = 1.0f;
			transform.m_translation = it.position;

			CreateActorData* data = new CreateActorData(transform, it.sid, itActor->m_mesh, itActor->m_material, 2.0f, i);

			RenderUpdateTaskType type = RenderUpdateTaskType::CreateActorGpuIndex;

			std::size_t address = (std::size_t)data;

			m_renderAspect->queueUpdateTask(type, (u8*)&address, sizeof(std::size_t));

			auto controller = m_physicsAspect->createActor(transform.m_translation, data->getHeight());
			data->setPhysx(controller);

			vx::Message msg;
			msg.type = vx::MessageType::Ingame_Event;
			msg.code = (u32)IngameMessage::Physx_AddActor;
			msg.arg1.ptr = data;

			msgManager->addMessage(msg);
		}
	}

	auto time = timer.getTimeMs();

	++s_counter;

	s_time = (s_time * (s_counter - 1) + time) / s_counter;

	return TaskReturnType::Success;
}

f32 TaskSceneCreateActors::getTimeMs() const
{
	return s_time;
}