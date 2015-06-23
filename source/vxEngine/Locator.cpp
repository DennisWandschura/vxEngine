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
#include "Locator.h"
#include <vxLib/types.h>

vx::EventManager* Locator::s_pEventManager{ nullptr };
PhysicsAspect* Locator::s_pPhysicsAspect{ nullptr };
FileAspect* Locator::s_pFileAspect{ nullptr };
RenderAspect* Locator::s_pRenderAspect{nullptr};

void Locator::provide(vx::EventManager* p)
{
	s_pEventManager = p;
}

vx::EventManager* Locator::getEventManager()
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

void Locator::provide(FileAspect* p)
{
	s_pFileAspect = p;
}

FileAspect* Locator::getFileAspect()
{
	VX_ASSERT(s_pFileAspect);
	return s_pFileAspect;
}


void Locator::provide(RenderAspect* p)
{
	s_pRenderAspect = p;
}

RenderAspect* Locator::getRenderAspect()
{
	VX_ASSERT(s_pRenderAspect);
	return s_pRenderAspect;
}

void Locator::reset()
{
	s_pEventManager = nullptr;
	s_pPhysicsAspect = nullptr;
	s_pFileAspect = nullptr;
	s_pRenderAspect = nullptr;
}