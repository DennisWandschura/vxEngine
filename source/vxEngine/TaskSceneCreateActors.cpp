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
#include <vxEngineLib/Graphics/RenderAspectInterface.h>
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
#include "PhysicsDefines.h"
#include <vxEngineLib/RendererMessage.h>
#include <vxEngineLib/ResourceAspectInterface.h>

TaskSceneCreateActors::TaskSceneCreateActors(const Event &evt, std::vector<Event> &&events, const Scene* scene,
	RenderAspectInterface* renderAspect, PhysicsAspect* physicsAspect, ResourceAspectInterface* resourceAspect)
	:Task(evt, std::move(events)),
	m_scene(scene),
	m_renderAspect(renderAspect),
	m_physicsAspect(physicsAspect),
	m_resourceAspect(resourceAspect)
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

		vx::Transform transform;
		transform.m_qRotation = vx::float4(0, 0, 0, 1);
		transform.m_scaling = 1.0f;
		transform.m_translation = it.position;

		CreateActorDataDesc desc;
		desc.height = g_heightStanding;
		desc.transform = transform;
		desc.spawnIndex = i;
		desc.type = it.type;
		desc.actorSid = it.actorSid;

		f32 height = g_heightStanding;
		f32 radius = 0.2f;
		if (it.type != PlayerType::Human)
		{
			auto actor = m_resourceAspect->getActor(it.actorSid);
			VX_ASSERT(actor != nullptr);

			desc.meshSid = actor->m_mesh;
			desc.materialSid = actor->m_material;;
			desc.m_fovRad = actor->m_fovRad;
			desc.m_maxViewDistance = actor->m_maxViewDistance;

			height = 2.0f;
			radius = 0.1f;
		}

		CreateActorData* data = new CreateActorData(desc);

		if (it.type != PlayerType::Human)
		{
			vx::Message msg;
			msg.type = vx::MessageType::Renderer;
			msg.code = (u32)vx::RendererMessage::AddActor;
			msg.arg1.ptr = data;

			msgManager->addMessage(msg);
		}
		
		auto controller = m_physicsAspect->createActor(transform.m_translation, height, radius);
		data->setPhysx(controller);

		vx::Message msg;
		msg.type = vx::MessageType::Ingame;
		msg.code = (u32)IngameMessage::Physx_AddedActor;
		msg.arg1.ptr = data;

		msgManager->addMessage(msg);
	}

	auto time = timer.getTimeMiliseconds();


	return TaskReturnType::Success;
}

f32 TaskSceneCreateActors::getTimeMs() const
{
	return 0.15f;
}