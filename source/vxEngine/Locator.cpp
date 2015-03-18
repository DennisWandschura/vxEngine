#include "Locator.h"
#include <vxLib/types.h>

EventManager* Locator::s_pEventManager{ nullptr };
PhysicsAspect* Locator::s_pPhysicsAspect{ nullptr };

void Locator::provide(EventManager* p)
{
	s_pEventManager = p;
}

EventManager* Locator::getEventManager()
{
	VX_ASSERT(s_pEventManager, "");
	return s_pEventManager;
}

void Locator::provide(PhysicsAspect* p)
{
	s_pPhysicsAspect = p;
}

PhysicsAspect* Locator::getPhysicsAspect()
{
	VX_ASSERT(s_pPhysicsAspect, "");
	return s_pPhysicsAspect;
}

void Locator::reset()
{
	s_pEventManager = nullptr;
	s_pPhysicsAspect = nullptr;
}