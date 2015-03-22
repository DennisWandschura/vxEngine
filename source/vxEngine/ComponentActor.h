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
		enum Flags : U16
		{
			HasDestination = 1 << 0,
			ReachedDestination = 1 <<1,
			HasPath = 1 << 2,

			WaitingForOrders = 1 << 8
		};

		U16 flags;
		StateMachine m_sm;
		std::unique_ptr<ActorData> data;
		F32 halfHeight;
		U16 evtMask;
	};
}