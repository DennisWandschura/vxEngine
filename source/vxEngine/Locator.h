#pragma once

class EventManager;
class PhysicsAspect;

class Locator
{
	static EventManager* s_pEventManager;
	static PhysicsAspect* s_pPhysicsAspect;

public:
	static void provide(EventManager* p);
	static EventManager* getEventManager();

	static void provide(PhysicsAspect* p);
	static PhysicsAspect* getPhysicsAspect();

	static void reset();
};