#pragma once

#include <vxLib/types.h>
#include "thread.h"
#include <atomic>
#include <vxLib/Allocator/StackAllocator.h>
#include "SystemAspect.h"
#include "RenderAspect.h"
#include "PhysicsAspect.h"
#include "FileAspect.h"
#include "memory.h"
#include "LevelEditor.h"
#include "EntityAspect.h"
#include "Profiler2.h"
#include "ProfilerGraph.h"
#include "EventManager.h"
#include "ActorAspect.h"

class Engine
{
	Profiler2 m_profiler;
	ProfilerGraph m_profileGraph;
	EventManager m_eventManager;
	SystemAspect m_systemAspect;
	PhysicsAspect m_physicsAspect;
	ActorAspect m_actorAspect;
	RenderAspect m_renderAspect;
	EntityAspect m_entityAspect;
	U32 m_bRun;
	VX_ALIGN(64) struct
	{
		FileAspect m_fileAspect;
		std::atomic_uint m_bRunFileThread;
	};
	vx::thread m_fileAspectThread;
	vx::StackAllocator m_allocator;
	U32 m_shutdown{0};
	Memory m_memory;

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void mainLoop();

public:
	Engine();
	~Engine();

	bool initialize();
	void shutdown();

	void start();

	void stop();

	void handleEvent(const Event &evt);
	void keyPressed(U16 key);
	void handleInput(const vx::Mouse &m, const vx::Keyboard &k, F32 dt);

	void requestLoadFile(const FileEntry &fileEntry, void* p);
};