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

namespace vx
{
	struct Message;
	class StackAllocator;
	class AllocationProfiler;
}

class Scene;

#include <vxLib/types.h>
#include <vxEngineLib/MessageListener.h>
#include <vxEngineLib/InfluenceMap.h>
#include <vxEngineLib/NavMeshGraph.h>
#include "ai/Squad.h"

class ActorAspect : public vx::MessageListener
{
	ai::Squad m_squad;
	InfluenceMap m_influenceMap;
	NavMeshGraph m_navmeshGraph;

	///////////////////
	// Event functions
	///////////////////

	void handleFileEvent(const vx::Message &evt);
	void handleIngameMessage(const vx::Message &evt);

	///////////////////

	void createInfluenceMap(const Scene* scene);

public:
	ActorAspect();

	void initialize(vx::StackAllocator* allocator, vx::AllocationProfiler* allocationManager);
	void shutdown();

	void handleMessage(const vx::Message &evt);

	void update();
};