#pragma once

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

namespace Component
{
	struct Actor;
}

namespace vx
{
	class StackAllocator;
}

struct EntityActor;
struct EntityHuman;
class RenderAspectInterface;
class QuadTree;
class ActionManager;

#include <vxEngineLib/Pool.h>

struct CreateActorComponentDesc
{
	u16 entityIndex;
	EntityHuman* human;
	EntityActor* entity;
	const QuadTree* quadTree; 
	u16* componentIndex;
	f32 fovRad;
	f32 maxViewDistance;
};

class ComponentActorManager
{
	vx::Pool<Component::Actor> m_pool;

public:
	ComponentActorManager();
	~ComponentActorManager();

	void initialize(u32 capacity, vx::StackAllocator* pAllocator);
	void shutdown();

	Component::Actor* createComponent(const CreateActorComponentDesc &desc);

	void update(ActionManager* actionManager, vx::StackAllocator* scratchAllocator);

	Component::Actor& operator[](u32 i);
	const Component::Actor& operator[](u32 i) const;
};