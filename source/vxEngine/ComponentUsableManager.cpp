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

#include "ComponentUsableManager.h"
#include <vxLib/Allocator/StackAllocator.h>
#include "ComponentUsable.h"
#include <vxEngineLib/MeshInstance.h>
#include <vxEngineLib/Locator.h>
#include <vxResourceAspect/ResourceAspect.h>
#include <vxEngineLib/Animation.h>

ComponentUsableManager::ComponentUsableManager()
	:m_poolUsable()
{

}

ComponentUsableManager::~ComponentUsableManager()
{

}

void ComponentUsableManager::initialize(u32 capacity, vx::StackAllocator* pAllocator)
{
	m_poolUsable.initialize(pAllocator->allocate(sizeof(Component::Usable) * capacity, __alignof(Component::Usable)), capacity);
}

void ComponentUsableManager::shutdown()
{
	m_poolUsable.clear();
	m_poolUsable.release();
}

Component::Usable* ComponentUsableManager::createComponent(const MeshInstance &instance, u16 entityIndex, u16* index)
{
	auto componentUsable = m_poolUsable.createEntry(index);

	auto animSid = instance.getAnimationSid();
	
	auto resourceAspect = Locator::getResourceAspect();
	auto animation = resourceAspect->getAnimation(animSid);

	componentUsable->action = nullptr;
	componentUsable->entityIndex = entityIndex;

	return componentUsable;
}

void ComponentUsableManager::update()
{
	auto physicsAspect = Locator::getPhysicsAspect();

	auto p = m_poolUsable.first();
	while (p != nullptr)
	{


		p = m_poolUsable.next_nocheck(p);
	}
}