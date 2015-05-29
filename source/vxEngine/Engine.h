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
#include "EventManager.h"
#include "ActorAspect.h"
#if _VX_AUDIO
#include <vxAudio/AudioAspect.h>
#endif

class Engine
{
	EventManager m_eventManager;
	SystemAspect m_systemAspect;
	PhysicsAspect m_physicsAspect;
	ActorAspect m_actorAspect;
	RenderAspect m_renderAspect;
	EntityAspect m_entityAspect;
	u32 m_bRun;
	FileAspect m_fileAspect;
	std::atomic_uint m_bRunFileThread;
	std::atomic_uint m_bRunRenderThread;
	vx::StackAllocator m_allocator;
	u32 m_shutdown{0};
	vx::thread m_fileAspectThread;
	vx::thread m_renderThread;
#if _VX_AUDIO
	vx::AudioAspect m_audioAspect;
#endif
	Memory m_memory;

	void loopFileThread();
	bool initializeImpl(const std::string &dataDir);

	void update();
	void mainLoop();
	void renderLoop();

public:
	Engine();
	~Engine();

	bool initialize();
	void shutdown();

	void start();

	void stop();

	void handleEvent(const vx::Event &evt);
	void keyPressed(u16 key);

	void requestLoadFile(const FileEntry &fileEntry, void* p);
};