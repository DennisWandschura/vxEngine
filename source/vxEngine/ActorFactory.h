#pragma once

namespace Component
{
	struct Actor;
	struct Input;
	struct Physics;
}

namespace vx
{
	class PoolAllocator;
}

class NavGraph;

#include <vxLib/types.h>

struct ActorFactory
{
	static void create(const NavGraph* navGraph, Component::Actor *p, Component::Input *pInput, Component::Physics *pPhysics, F32 halfHeight, vx::PoolAllocator* pAllocator);
};