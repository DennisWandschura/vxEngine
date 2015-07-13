#include "TaskSceneCreateStaticMeshes.h"
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/RenderAspectInterface.h>
#include <vxEngineLib/Material.h>

TaskSceneCreateStaticMeshes::TaskSceneCreateStaticMeshes(const Scene* scene, RenderAspectInterface* renderAspect)
	:m_scene(scene),
	m_renderAspect(renderAspect)
{

}

TaskSceneCreateStaticMeshes::~TaskSceneCreateStaticMeshes()
{

}

void TaskSceneCreateStaticMeshes::run()
{
	auto instanceCount = m_scene->getMeshInstanceCount();
	auto instances = m_scene->getMeshInstances();
	for (u32 i = 0; i < instanceCount; ++i)
	{
		auto &instance = instances[i];

		RenderUpdateTask task;
		task.type = RenderUpdateTask::Type::CreateStaticMesh;

		RenderUpdateTaskCreateStaticMeshData data;
		data.instance = &instance;
		data.materialSid = instance.getMaterial()->getSid();

		m_renderAspect->queueUpdateTask(task, (u8*)&data, sizeof(RenderUpdateTaskCreateStaticMeshData));
	}
}

bool TaskSceneCreateStaticMeshes::isFinished() const
{
	return true;
}

Task* TaskSceneCreateStaticMeshes::move(vx::Allocator* allocator)
{
	auto ptr = (TaskSceneCreateStaticMeshes*)allocator->allocate(sizeof(TaskSceneCreateStaticMeshes), __alignof(TaskSceneCreateStaticMeshes));

	ptr->m_scene = m_scene;
	ptr->m_renderAspect = m_renderAspect;

	return ptr;
}