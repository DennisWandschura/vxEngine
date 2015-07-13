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

#include "ComponentPhysicsManager.h"
#include "ComponentPhysics.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <characterkinematic/PxController.h>
#include <vxEngineLib/Entity.h>

ComponentPhysicsManager::ComponentPhysicsManager()
	:m_poolPhysics()
{

}

ComponentPhysicsManager::~ComponentPhysicsManager()
{

}

void ComponentPhysicsManager::initialize(u32 capacity, vx::StackAllocator* pAllocator)
{
	m_poolPhysics.initialize(pAllocator->allocate(sizeof(Component::Physics) * capacity, __alignof(Component::Physics)), capacity);
}

void ComponentPhysicsManager::shutdown()
{
	m_poolPhysics.clear();
	m_poolPhysics.release();
}

Component::Physics* ComponentPhysicsManager::createComponent(const vx::float3 &position, physx::PxController* controller, u16 entityIndex, u16* componentIndex)
{
	auto ptr = m_poolPhysics.createEntry(componentIndex);

	ptr->entityIndex = entityIndex;
	ptr->footPositionY = 0;
	ptr->position = position;
	ptr->controller = controller;

	return ptr;
}

void ComponentPhysicsManager::update(vx::Pool<Entity>* entities)
{
	auto p = m_poolPhysics.first();
	while (p != nullptr)
	{
		auto &entity = (*entities)[p->entityIndex];
		//auto contactOffset = p->pRigidActor->getContactOffset();
		auto footPosition = p->controller->getFootPosition();
		auto position = p->controller->getPosition();

		p->position.x = position.x;
		p->position.y = position.y;
		p->position.z = position.z;
		p->footPositionY = footPosition.y;

		entity.position = p->position;

		p = m_poolPhysics.next_nocheck(p);
	}
}

Component::Physics& ComponentPhysicsManager::operator[](u32 i)
{
	return m_poolPhysics[i];
}