#include "TaskSceneCreateActorsGpu.h"
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/RenderAspectInterface.h>
#include <vxEngineLib/Spawn.h>
#include <vxEngineLib/Actor.h>
#include <vxEngineLib/CreateActorData.h>
#include <vxEngineLib/Locator.h>
#include <vxEngineLib/Event.h>
#include <vxEngineLib/EventManager.h>
#include <vxEngineLib/EventsIngame.h>
#include <vxEngineLib/EventTypes.h>

TaskSceneCreateActorsGpu::TaskSceneCreateActorsGpu(const Scene* scene, RenderAspectInterface* renderAspect)
	:m_scene(scene),
	m_renderAspect(renderAspect)
{

}

TaskSceneCreateActorsGpu::~TaskSceneCreateActorsGpu()
{

}

void TaskSceneCreateActorsGpu::run()
{
	auto spawns = m_scene->getSpawns();
	auto spawnCount = m_scene->getSpawnCount();

	auto evtManager = Locator::getEventManager();

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

			RenderUpdateTask task;
			task.type = RenderUpdateTask::Type::CreateActorGpuIndex;

			std::size_t address = (std::size_t)data;

			m_renderAspect->queueUpdateTask(task, (u8*)&address, sizeof(std::size_t));

			vx::Event evt;
			evt.type = vx::EventType::Ingame_Event;
			evt.code = (u32)IngameEvent::Physx_AddActor;
			evt.arg1.ptr = data;

			evtManager->addEvent(evt);
		}
	}
}

bool TaskSceneCreateActorsGpu::isFinished() const
{
	return true;
}

Task* TaskSceneCreateActorsGpu::move(vx::Allocator* allocator)
{
	auto ptr = (TaskSceneCreateActorsGpu*)allocator->allocate(sizeof(TaskSceneCreateActorsGpu), __alignof(TaskSceneCreateActorsGpu));

	ptr->m_scene = m_scene;
	ptr->m_renderAspect = m_renderAspect;

	return ptr;
}