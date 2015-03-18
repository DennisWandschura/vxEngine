#include "EntityFactory.h"
#include "ActorFactory.h"

void EntityFactory::create(const NavGraph* navGraph, Component::Actor *p, Component::Input *pInput, Component::Physics *pPhysics, F32 halfHeight, vx::PoolAllocator* pAllocator)
{
	ActorFactory::create(navGraph, p, pInput, pPhysics, halfHeight, pAllocator);
}