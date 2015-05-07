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
#pragma once

class DecisionTreeNode;
class NavGraph;

#include "Component.h"
#include "StateMachine.h"
#include <vxLib/math/Vector.h>
#include <memory>
#include <vxLib/Container/array.h>

struct ActorData
{
	// capacity is count of nav nodes
	vx::array<vx::float3> path;
	vx::float3 destination;
};

namespace Component
{
	struct Actor : public Base
	{
		enum Flags : u16
		{
			HasDestination = 1 << 0,
			ReachedDestination = 1 <<1,
			HasPath = 1 << 2,

			WaitingForOrders = 1 << 8
		};

		u16 flags;
		StateMachine m_sm;
		std::unique_ptr<ActorData> data;
		f32 halfHeight;
		u16 evtMask;
	};
}