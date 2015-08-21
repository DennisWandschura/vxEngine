#include "ComponentRenderManager.h"
#include "ComponentRender.h"
#include <vxEngineLib/GpuFunctions.h>
#include <vxLib/ScopeGuard.h>
#include <vxEngineLib/Transform.h>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxEngineLib/RenderAspectInterface.h>
#include "Entity.h"

ComponentRenderManager::ComponentRenderManager()
	:m_poolRender()
{

}

ComponentRenderManager::~ComponentRenderManager()
{

}

void ComponentRenderManager::initialize(u32 capacity, vx::StackAllocator* pAllocator)
{
	m_poolRender.initialize(pAllocator->allocate(sizeof(Component::Render) * capacity, __alignof(Component::Render)), capacity);
}

void ComponentRenderManager::shutdown()
{
	m_poolRender.clear();
	m_poolRender.release();
}

Component::Render* ComponentRenderManager::createComponent(const vx::Transform &transform, u32 gpuIndex, u16 entityIndex, u16* index)
{
	auto ptr = m_poolRender.createEntry(index);
	ptr->entityIndex = entityIndex;
	ptr->gpuIndex = gpuIndex;
	//ptr->translation = transform.m_translation;
	//ptr->qRotation = vx::loadFloat4(transform.m_qRotation);

	return ptr;
}

void ComponentRenderManager::update(vx::StackAllocator* scratchAllocator, RenderAspectInterface* renderAspect, const vx::Pool<EntityActor> &entities)
{
	s32 count = m_poolRender.size();

	auto marker = scratchAllocator->getMarker();
	SCOPE_EXIT
	{
		scratchAllocator->clear(marker);
	};

	auto totalSizeInBytes = sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count + sizeof(u32) * count;
	auto dataPtr = scratchAllocator->allocate(totalSizeInBytes, 16);

	vx::TransformGpu* pTransforms = (vx::TransformGpu*)(dataPtr + sizeof(RenderUpdateDataTransforms));
	u32* indices = (u32*)(dataPtr + sizeof(RenderUpdateDataTransforms) + sizeof(vx::TransformGpu) * count);
	u32 index = 0;

	auto p = m_poolRender.first();
	while (p != nullptr)
	{
		auto data = *p;

		auto &entity = entities[data.entityIndex];

		__m128 qRotation = vx::loadFloat4(entity.m_qRotation);
		auto translation = entity.m_position;

		auto packedRotation = GpuFunctions::packQRotation(qRotation);

		vx::TransformGpu transform;
		transform.translation = translation;
		transform.scaling = 1.0f;
		transform.packedQRotation = packedRotation;

		indices[index] = data.gpuIndex;
		pTransforms[index] = transform;
		++index;

		p = m_poolRender.next_nocheck(p);
	}

	RenderUpdateDataTransforms* renderUpdateData = (RenderUpdateDataTransforms*)dataPtr;
	renderUpdateData->count = count;

	RenderUpdateTaskType type = RenderUpdateTaskType::UpdateDynamicTransforms;

	renderAspect->queueUpdateTask(type, dataPtr, totalSizeInBytes);
}

Component::Render& ComponentRenderManager::operator[](u32 i)
{
	return m_poolRender[i];
}

const Component::Render& ComponentRenderManager::operator[](u32 i) const
{
	return m_poolRender[i];
}