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
	struct Input;
}

namespace vx
{
	class StackAllocator;
}

struct Entity;
struct QuadTreeData;
class ComponentPhysicsManager;

#include <vxEngineLib/Pool.h>
#include <vector>

class ComponentInputManager
{
	vx::Pool<Component::Input> m_poolInput;

public:
	ComponentInputManager();
	~ComponentInputManager();

	void initialize(u32 capacity, vx::StackAllocator* pAllocator);
	void shutdown();

	void update(f32 dt, ComponentPhysicsManager* componentPhysicsManager, vx::Pool<Entity>* entities);

	void getQuadTreeData(std::vector<QuadTreeData>* data, ComponentPhysicsManager* componentPhysicsManager, vx::Pool<Entity>* entities);

	Component::Input* createComponent(u16 entityIndex, u16* componentIndex);

	Component::Input& operator[](u32 i);

	u32 getSize() const;
};