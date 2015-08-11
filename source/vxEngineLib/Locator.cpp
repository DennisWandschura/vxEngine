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
#include <vxEngineLib/Locator.h>
#include <vxLib/types.h>

vx::MessageManager* Locator::s_pEventManager{ nullptr };
PhysicsAspect* Locator::s_pPhysicsAspect{ nullptr };
ResourceAspect* Locator::s_pResourceAspect{ nullptr };
RenderAspectInterface* Locator::s_pRenderAspect{nullptr};

void Locator::provide(vx::MessageManager* p)
{
	s_pEventManager = p;
}

vx::MessageManager* Locator::getMessageManager()
{
	VX_ASSERT(s_pEventManager);
	return s_pEventManager;
}

void Locator::provide(PhysicsAspect* p)
{
	s_pPhysicsAspect = p;
}

PhysicsAspect* Locator::getPhysicsAspect()
{
	VX_ASSERT(s_pPhysicsAspect);
	return s_pPhysicsAspect;
}

void Locator::provide(ResourceAspect* p)
{
	s_pResourceAspect = p;
}

ResourceAspect* Locator::getResourceAspect()
{
	VX_ASSERT(s_pResourceAspect);
	return s_pResourceAspect;
}

void Locator::provide(RenderAspectInterface* p)
{
	s_pRenderAspect = p;
}

RenderAspectInterface* Locator::getRenderAspect()
{
	VX_ASSERT(s_pRenderAspect);
	return s_pRenderAspect;
}

void Locator::reset()
{
	s_pEventManager = nullptr;
	s_pPhysicsAspect = nullptr;
	s_pResourceAspect = nullptr;
	s_pRenderAspect = nullptr;
}