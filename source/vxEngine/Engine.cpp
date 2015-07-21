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
#include "Engine.h"
#include <vxEngineLib/Timer.h>
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib\/Locator.h>
#include "developer.h"
#include "EngineGlobals.h"
#include "CpuProfiler.h"
#include <vxEngineLib/EventTypes.h>
#include <vxEngineLib/debugPrint.h>

Engine* g_pEngine{ nullptr };

namespace EngineCpp
{
	void callbackKeyPressed(u16 key)
	{
		g_pEngine->keyPressed(key);
	}
}

Engine::Engine()
	:m_eventManager(),
	m_actionManager(),
	m_taskManager(),
	m_systemAspect(),
	m_physicsAspect(),
	m_actorAspect(),
	m_renderAspect(nullptr),
	m_entityAspect(),
	m_bRun(0),
	m_fileAspect(),
	m_bRunFileThread(),
#if _VX_MEM_PROFILE
	m_allocManager(),
#endif
	m_allocator(),
	m_shutdown(0),
	m_fileAspectThread(),
	m_scene(),
	m_renderAspectDll(nullptr),
	m_destroyFn(nullptr)
{
	g_pEngine = this;
}

Engine::~Engine()
{
	if (m_shutdown == 0)
	{
		// something bad happened
		assert(false);
	}
}

void Engine::update()
{
	//printf("physx simulate\n");
	m_physicsAspect.update(g_dt);

	// process events
	m_eventManager.update();

	m_actionManager.update();

	//printf("task manager begin\n");
	m_taskManager.update();
	m_taskManager.wait();
	//printf("task manager end\n");

	// update aspects in order
	//printf("physx fetch\n");
	m_physicsAspect.fetch();

	m_systemAspect.update(g_dt);

	m_entityAspect.update(g_dt, &m_actionManager);

	//CpuProfiler::popMarker();

	//CpuProfiler::pushMarker("physx");
	//CpuProfiler::popMarker();
}

void Engine::mainLoop()
{
	auto frequency = Timer::getFrequency();
	const f64 invFrequency = 1.0 / frequency;

	LARGE_INTEGER last;
	QueryPerformanceCounter(&last);

	m_renderAspect->initializeProfiler();

	f32 accum = 0.0f;
	while (m_bRun != 0)
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		auto frameTicks = (current.QuadPart - last.QuadPart) * 1000;
		f32 frameTime = frameTicks * invFrequency * 0.001f;
		frameTime = fminf(frameTime, g_dt);

		accum += frameTime;

		m_renderAspect->update();
		m_renderAspect->submitCommands();

		while (accum >= g_dt)
		{
			update();

			m_renderAspect->updateProfiler(g_dt);

			accum -= g_dt;
		}

		m_renderAspect->endFrame();

		last = current;
	}
}

void Engine::loopFileThread()
{
	while (m_bRunFileThread.load() != 0)
	{
		// do work
		m_fileAspect.update();

		// sleep for a bit
		std::this_thread::yield();
	}
}

bool Engine::initializeImpl(const std::string &dataDir)
{
	m_memory = Memory(100 MBYTE, 64);

	m_allocator = vx::StackAllocator(m_memory.get(), m_memory.size());

#if _VX_MEM_PROFILE
	m_allocManager.registerAllocator(&m_allocator, "mainAllocator");
#endif

	m_eventManager.initialize(&m_allocator, 255);
	Locator::provide(&m_eventManager);

	if (!m_fileAspect.initialize(&m_allocator, dataDir, &m_eventManager, nullptr))
		return false;

	return true;
}

bool Engine::createRenderAspectGL(const RenderAspectDescription &desc)
{
	auto handle = LoadLibrary(L"../../lib/vxRenderAspectGL_d.dll");
	if (handle == nullptr)
		return false;

	auto proc = (CreateRenderAspectFunction)GetProcAddress(handle, "createRenderAspect");
	auto procDestroy = (DestroyRenderAspectFunction)GetProcAddress(handle, "destroyRenderAspect");
	if (proc == nullptr || procDestroy == nullptr)
		return false;

	RenderAspectInitializeError error;
	auto renderAspect = proc(desc, &error);
	if (renderAspect == nullptr)
		return false;

	m_renderAspect = renderAspect;
	m_renderAspectDll = handle;
	m_destroyFn = procDestroy;

	return true;
}

bool Engine::createRenderAspectDX12(const RenderAspectDescription &desc)
{
	auto handle = LoadLibrary(L"../../lib/vxRenderAspectDX12_d.dll");
	if (handle == nullptr)
		return false;

	auto proc = (CreateRenderAspectFunction)GetProcAddress(handle, "createRenderAspect");
	auto procDestroy = (DestroyRenderAspectFunction)GetProcAddress(handle, "destroyRenderAspect");
	if (proc == nullptr || procDestroy == nullptr)
		return false;

	auto renderAspect = proc(desc, nullptr);
	if (renderAspect == nullptr)
		return false;

	m_renderAspect = renderAspect;
	m_renderAspectDll = handle;
	m_destroyFn = procDestroy;

	return true;
}

bool Engine::initialize()
{
	const std::string dataDir("../data/");

#if _VX_MEM_PROFILE
	vx::Allocator::setProfiler(&m_allocManager);
#endif

	if (!g_engineConfig.loadFromFile("settings.txt"))
	{
		printf("could not load 'settings.txt'\n");
		return false;
	}

	g_engineConfig.m_editor = false;

	if (!initializeImpl(dataDir))
		return false;

#if _VX_MEM_PROFILE
	m_taskManager.initialize(&m_allocator, 1024, &m_allocManager);
#else
	m_taskManager.initialize(&m_allocator, 1024, nullptr);
#endif

	if (!m_systemAspect.initialize(g_engineConfig, EngineCpp::callbackKeyPressed, nullptr))
		return false;

	RenderAspectDescription renderAspectDesc =
	{
		dataDir,
		(void*)&m_systemAspect.getWindow(),
		nullptr,
		&m_allocator,
		&g_engineConfig,
		&m_fileAspect,
		&m_eventManager,
	};

	auto renderMode = g_engineConfig.m_rendererSettings.m_renderMode;
	if (renderMode == Graphics::RendererSettings::Mode_GL)
	{
		if (!createRenderAspectGL(renderAspectDesc))
		{
			return false;
		}
	}
	else if (renderMode == Graphics::RendererSettings::Mode_DX12)
	{
		if (!createRenderAspectDX12(renderAspectDesc))
		{
			return false;
		}
	}

	//m_renderAspect->makeCurrent(false);

	if (!m_physicsAspect.initialize(&m_taskManager))
		return false;

#if _VX_MEM_PROFILE
	if (!m_entityAspect.initialize(&m_allocator, &m_taskManager, &m_allocManager))
		return false;
#else
	if (!m_entityAspect.initialize(&m_allocator, &m_taskManager, nullptr))
		return false;
#endif

#if _VX_AUDIO
	if (!m_audioAspect.initialize())
		return false;
#endif

#if _VX_MEM_PROFILE
	m_actorAspect.initialize(&m_allocator, &m_allocManager);
#else
	m_actorAspect.initialize(&m_allocator, nullptr);
#endif

	Locator::provide(&m_physicsAspect);
	Locator::provide(m_renderAspect);
	Locator::provide(&m_fileAspect);

	// register aspects that receive events
	m_eventManager.registerListener(m_renderAspect, 3, (u8)vx::EventType::File_Event);
	m_eventManager.registerListener(&m_physicsAspect, 2, (u8)vx::EventType::File_Event | (u8)vx::EventType::Ingame_Event);
	m_eventManager.registerListener(&m_entityAspect, 1, (u8)vx::EventType::File_Event | (u8)vx::EventType::Ingame_Event);
	m_eventManager.registerListener(&m_actorAspect, 2, (u8)vx::EventType::File_Event | (u8)vx::EventType::Ingame_Event | (u8)vx::EventType::AI_Event);

	m_bRun = 1;
	m_bRunFileThread.store(1);
	m_bRunRenderThread.store(1);
	m_shutdown = 0;

	return true;
}

void Engine::shutdown()
{
	m_fileAspectThread.join();
	//m_renderThread.join();

	m_scene.reset();

#if _VX_AUDIO
	m_audioAspect.shutdown();
#endif
	m_entityAspect.shutdown();
	m_physicsAspect.shutdown();

	if (m_renderAspect)
	{
		m_renderAspect->shutdown(m_systemAspect.getWindow().getHwnd());
		m_destroyFn(m_renderAspect);
		m_renderAspect = nullptr;
		m_destroyFn = nullptr;
	}

	m_systemAspect.shutdown();
	m_fileAspect.shutdown();
	m_shutdown = 1;

	Locator::reset();

	m_taskManager.shutdown();

	//CpuProfiler::shutdown();

	m_allocator.clear();
	m_allocator.release();

#if _VX_MEM_PROFILE
	m_allocManager.print();
#endif

	if (m_renderAspectDll != nullptr)
	{
		FreeLibrary(m_renderAspectDll);
		m_renderAspectDll = nullptr;
	}

	m_memory.clear();
}

void Engine::start()
{
	m_fileAspectThread = vx::thread(&Engine::loopFileThread, this);

	//RenderAspectThreadDesc desc{ &m_systemAspect.getWindow(), &g_engineConfig};
	//m_renderThread = vx::thread(&Engine::renderLoop, this, desc);

	std::string level;
	g_engineConfig.m_root.get("level")->as(&level);

	requestLoadFile(vx::FileEntry(level.c_str(), vx::FileType::Scene), &m_scene);

	mainLoop();
}

void Engine::stop()
{
	m_bRun = 0;
	m_bRunFileThread.store(0);
	m_bRunRenderThread.store(0);
}

void Engine::requestLoadFile(const vx::FileEntry &fileEntry, void* p)
{
	m_fileAspect.requestLoadFile(fileEntry, p);
}

void Engine::keyPressed(u16 key)
{
	if (key == vx::Keyboard::Key_E)
	{
		m_entityAspect.onPressedActionKey();
	}
	else if (key == vx::Keyboard::Key_Escape)
	{
		stop();
	}
}