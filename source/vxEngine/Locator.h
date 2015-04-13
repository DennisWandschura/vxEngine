#pragma once

class EventManager;
class PhysicsAspect;
class FileAspect;

class Locator
{
	static EventManager* s_pEventManager;
	static PhysicsAspect* s_pPhysicsAspect;
	static FileAspect* s_pFileAspect;

public:
	static void provide(EventManager* p);
	static EventManager* getEventManager();

	static void provide(PhysicsAspect* p);
	static PhysicsAspect* getPhysicsAspect();

	static void provide(FileAspect* p);
	static FileAspect* getFileAspect();

	static void reset();
};