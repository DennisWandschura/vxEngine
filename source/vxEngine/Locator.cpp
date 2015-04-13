#include "Locator.h"
#include <vxLib/types.h>

EventManager* Locator::s_pEventManager{ nullptr };
PhysicsAspect* Locator::s_pPhysicsAspect{ nullptr };
FileAspect* Locator::s_pFileAspect{ nullptr };

void Locator::provide(EventManager* p)
{
	s_pEventManager = p;
}

EventManager* Locator::getEventManager()
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
	VX_ASSERT(s_pPhysicsAspect);
	return s_pFileAspect;
}

void Locator::reset()
{
	s_pEventManager = nullptr;
	s_pPhysicsAspect = nullptr;
	s_pFileAspect = nullptr;
}