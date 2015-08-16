#pragma once
/* The MIT License (MIT)

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
SOFTWARE.*/

#include <vxLib/types.h>
#include <atomic>
#include <vxLib/Allocator/StackAllocator.h>
#include "SystemAspect.h"
#include <vxEngineLib/RenderAspectInterface.h>
#include "PhysicsAspect.h"
#include <vxResourceAspect/FileAspect.h>
#include "memory.h"
#include "LevelEditor.h"
#include "EntityAspect.h"
#include <vxEngineLib/MessageManager.h>
#include "ActorAspect.h"
#include <vxEngineLib/Scene.h>
#include "ActionManager.h"
#include <vxEngineLib/TaskManager.h>
#include <vxResourceAspect/ResourceAspect.h>
#if _VX_MEM_PROFILE
#include <vxLib/Allocator/AllocationProfiler.h>
#endif
#if _VX_AUDIO
#include <vxAudio/AudioAspect.h>
#endif

class Engine
{
	vx::MessageManager m_msgManager;
	ActionManager m_actionManager;
	vx::TaskManager m_taskManager;
	SystemAspect m_systemAspect;
	PhysicsAspect m_physicsAspect;
	ActorAspect m_actorAspect;
	RenderAspectInterface* m_renderAspect;
	EntityAspect m_entityAspect;
	u32 m_bRun;
	ResourceAspect m_resourceAspect;
#if _VX_MEM_PROFILE
	vx::AllocationProfiler m_allocManager;
#endif
	vx::StackAllocator m_allocator;
	u32 m_shutdown;
	//std::thread m_fileAspectThread;
	std::thread m_taskManagerThread;
	//vx::thread m_renderThread;
#if _VX_AUDIO
	vx::AudioAspect m_audioAspect;
#endif
	Scene m_scene;
	HMODULE m_renderAspectDll;
	DestroyRenderAspectFunction m_destroyFn;
	Memory m_memory;

	bool createRenderAspectGL(const RenderAspectDescription &desc);
	bool createRenderAspectDX12(const RenderAspectDescription &desc);

	bool initializeImpl(const std::string &dataDir);

	void update();
	void mainLoop();
	//void renderLoop(const RenderAspectThreadDesc &desc);

public:
	Engine();
	~Engine();

	bool initialize();
	void shutdown();

	void start();

	void stop();

	void handleMessage(const vx::Message &evt);
	void keyPressed(u16 key);

	void requestLoadFile(const vx::FileEntry &fileEntry, void* p);
};