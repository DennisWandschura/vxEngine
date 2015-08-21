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
	struct Physics;
}

namespace vx
{
	class StackAllocator;
}

namespace physx
{
	class PxController;
	class PxRigidDynamic;
}

struct EntityActor;

#include <vxEngineLib/Pool.h>
#include <vxLib/math/Vector.h>

class ComponentPhysicsManager
{
	vx::Pool<Component::Physics> m_poolPhysics;

public:
	ComponentPhysicsManager();
	~ComponentPhysicsManager();

	void initialize(u32 capacity, vx::StackAllocator* pAllocator);
	void shutdown();

	void update(vx::Pool<EntityActor>* entities);

	Component::Physics* createComponent(const vx::float3 &position, physx::PxController* controller, physx::PxRigidDynamic* rigidDynamic, u16 entityIndex, u16* componentIndex);

	Component::Physics& operator[](u32 i);
};