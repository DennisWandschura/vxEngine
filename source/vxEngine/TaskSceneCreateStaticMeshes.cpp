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
#include "TaskSceneCreateStaticMeshes.h"
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/RenderAspectInterface.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/CreateDynamicMeshData.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/EventsIngame.h>

TaskSceneCreateStaticMeshes::TaskSceneCreateStaticMeshes(u32 tid, const Scene* scene, RenderAspectInterface* renderAspect)
	:Task(tid),
	m_scene(scene),
	m_renderAspect(renderAspect)
{

}

TaskSceneCreateStaticMeshes::TaskSceneCreateStaticMeshes(TaskSceneCreateStaticMeshes &&rhs)
	:Task(std::move(rhs)),
	m_scene(rhs.m_scene),
	m_renderAspect(rhs.m_renderAspect)
{

}

TaskSceneCreateStaticMeshes::~TaskSceneCreateStaticMeshes()
{

}

TaskReturnType TaskSceneCreateStaticMeshes::run()
{
	auto evtManager = Locator::getEventManager();

	auto instanceCount = m_scene->getMeshInstanceCount();
	auto instances = m_scene->getMeshInstances();
	for (u32 i = 0; i < instanceCount; ++i)
	{
		auto &instance = instances[i];

		auto type = instance.getRigidBodyType();

		switch (type)
		{
		case PhysxRigidBodyType::Static:
		{
			RenderUpdateTaskType type = RenderUpdateTaskType::AddStaticMeshInstance;

			RenderUpdateTaskAddStaticMeshData data;
			data.instance = &instance;
			data.materialSid = instance.getMaterial()->getSid();

			m_renderAspect->queueUpdateTask(type, (u8*)&data, sizeof(RenderUpdateTaskAddStaticMeshData));

			vx::Event evt;
			evt.type = vx::EventType::Ingame_Event;
			evt.code = (u32)IngameEvent::Physx_AddStaticMesh;
			evt.arg1.ptr = (void*)&instance;

			evtManager->addEvent(evt);

		}break;
		case PhysxRigidBodyType::Dynamic:
		{
			auto data = new CreateDynamicMeshData();
			data->m_meshInstance = &instance;
			data->m_materialSid = instance.getMaterial()->getSid();

			std::size_t address = (std::size_t)data;

			RenderUpdateTaskType type = RenderUpdateTaskType::AddDynamicMeshInstance;
			m_renderAspect->queueUpdateTask(type, (u8*)&address, sizeof(address));

			vx::Event evt;
			evt.type = vx::EventType::Ingame_Event;
			evt.code = (u32)IngameEvent::Physx_AddDynamicMesh;
			evt.arg1.ptr = data;

			evtManager->addEvent(evt);
		}break;
		default:
			break;
		}
	}

	return TaskReturnType::Success;
}

Task* TaskSceneCreateStaticMeshes::move(vx::Allocator* allocator)
{
	auto ptr = (TaskSceneCreateStaticMeshes*)allocator->allocate(sizeof(TaskSceneCreateStaticMeshes), __alignof(TaskSceneCreateStaticMeshes));

	new (ptr) TaskSceneCreateStaticMeshes(std::move(*this));

	return ptr;
}