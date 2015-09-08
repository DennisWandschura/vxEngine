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
#include <vxEngineLib/Graphics/RenderAspectInterface.h>
#include <vxEngineLib/Material.h>
#include <vxEngineLib/CreateDynamicMeshData.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/MessageManager.h>
#include <vxEngineLib/Message.h>
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/IngameMessage.h>
#include <vxEngineLib/CpuTimer.h>
#include <vxEngineLib/RendererMessage.h>

thread_local f32 TaskSceneCreateStaticMeshes::s_time{0.0f};
thread_local u64 TaskSceneCreateStaticMeshes::s_counter{0};

TaskSceneCreateStaticMeshes::TaskSceneCreateStaticMeshes(const Scene* scene, RenderAspectInterface* renderAspect)
	:m_scene(scene),
	m_renderAspect(renderAspect)
{

}

TaskSceneCreateStaticMeshes::~TaskSceneCreateStaticMeshes()
{

}

TaskReturnType TaskSceneCreateStaticMeshes::runImpl()
{
	CpuTimer timer;

	auto evtManager = Locator::getMessageManager();

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

			vx::Message evt;

			evt.type = vx::MessageType::Renderer;
			evt.code = (u32)vx::RendererMessage::AddStaticMesh;
			evt.arg1.ptr = (void*)&instance;
			evt.arg2.u64 = instance.getMaterial()->getSid().value;
			evtManager->addMessage(evt);

		
			evt.type = vx::MessageType::Ingame;
			evt.code = (u32)IngameMessage::Physx_AddStaticMesh;
			evt.arg1.ptr = (void*)&instance;

			evtManager->addMessage(evt);

		}break;
		case PhysxRigidBodyType::Dynamic:
		{
			auto data = new CreateDynamicMeshData();
			data->initialize(&instance, instance.getMaterial()->getSid());

			vx::Message evt;

			evt.type = vx::MessageType::Renderer;
			evt.code = (u32)vx::RendererMessage::AddDynamicMesh;
			evt.arg1.ptr = data;
			evtManager->addMessage(evt);

			evt.type = vx::MessageType::Ingame;
			evt.code = (u32)IngameMessage::Physx_AddDynamicMesh;
			evt.arg1.ptr = data;
			evtManager->addMessage(evt);
		}break;
		default:
			break;
		}
	}

	++s_counter;
	auto time = timer.getTimeMs();

	s_time = (s_time * (s_counter - 1) + time) / s_counter;

	return TaskReturnType::Success;
}

f32 TaskSceneCreateStaticMeshes::getTimeMs() const
{
	return s_time;
}