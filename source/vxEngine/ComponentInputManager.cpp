#include "ComponentInputManager.h"
#include <vxEngineLib/Locator.h>
#include "ComponentInput.h"
#include "PhysicsDefines.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "QuadTree.h"
#include "ComponentPhysicsManager.h"
#include "ComponentPhysics.h"
#include <vxEngineLib/Entity.h>
#include "PhysicsAspect.h"

ComponentInputManager::ComponentInputManager()
	:m_poolInput()
{

}

ComponentInputManager::~ComponentInputManager()
{

}

void ComponentInputManager::initialize(u32 capacity, vx::StackAllocator* pAllocator)
{
	m_poolInput.initialize(pAllocator->allocate(sizeof(Component::Input) * capacity, __alignof(Component::Input)), capacity);
}

void ComponentInputManager::shutdown()
{
	m_poolInput.clear();
	m_poolInput.release();
}

void ComponentInputManager::update(f32 dt, ComponentPhysicsManager* componentPhysicsManager, vx::Pool<Entity>* entities)
{
	const vx::ivec4 velocityMask = { (s32)0xffffffff, 0, (s32)0xffffffff, 0 };
	auto physicsAspect = Locator::getPhysicsAspect();

	const __m128 vGravity = { 0, g_gravity * dt, 0, 0 };
	auto p = m_poolInput.first();
	while (p != nullptr)
	{
		auto &entity = (*entities)[p->entityIndex];

		__m128 qRotation = { p->orientation.y, p->orientation.x, 0, 0 };
		qRotation = vx::quaternionRotationRollPitchYawFromVector(qRotation);
		vx::storeFloat4(&entity.qRotation, qRotation);

		//__m128 vVelocity = { p->velocity.x, 0, p->velocity.z, 0.0f };
		__m128 vVelocity = vx::loadFloat4(p->velocity);

		vVelocity = _mm_and_ps(vVelocity, velocityMask);
		vVelocity = _mm_add_ps(vVelocity, vGravity);

		u8 index = 0xff;
		entity.getComponentIndex<Component::Physics>(&index);
		auto &physics = (*componentPhysicsManager)[index];

		physicsAspect->move(vVelocity, dt, physics.controller);

		p = m_poolInput.next_nocheck(p);
	}
}

void ComponentInputManager::getQuadTreeData(std::vector<QuadTreeData>* data, ComponentPhysicsManager* componentPhysicsManager, vx::Pool<Entity>* entities)
{
	auto current = m_poolInput.first();
	while (current != nullptr)
	{
		auto &entity = (*entities)[current->entityIndex];

		u8 physicsIndex;
		entity.getComponentIndex<Component::Physics>(&physicsIndex);

		auto componentPhysics = (*componentPhysicsManager)[physicsIndex];

		QuadTreeData tmp;
		tmp.entity = &entity;
		tmp.position = entity.position;
		tmp.position.y = componentPhysics.footPositionY;
		tmp.velocity.x = current->velocity.x;
		tmp.velocity.y = current->velocity.y;
		tmp.velocity.z = current->velocity.z;

		data->push_back(tmp);

		current = m_poolInput.next_nocheck(current);
	}
}

Component::Input* ComponentInputManager::createComponent(u16 entityIndex, u16* componentIndex)
{
	auto ptr = m_poolInput.createEntry(componentIndex);
	ptr->entityIndex = entityIndex;
	ptr->orientation.x = 0;

	return ptr;
}

Component::Input& ComponentInputManager::operator[](u32 i)
{
	return m_poolInput[i];
}

u32 ComponentInputManager::getSize() const
{
	return m_poolInput.size();
}