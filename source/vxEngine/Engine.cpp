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
//#include "DebugRenderSettings.h"
#include "EngineGlobals.h"
#include "CpuProfiler.h"
#include <vxEngineLib/EventTypes.h>

Engine* g_pEngine{ nullptr };

namespace
{
	void callbackKeyPressed(u16 key)
	{
		g_pEngine->keyPressed(key);
	}
}

Engine::Engine()
	:m_eventManager(),
	m_systemAspect(),
	m_physicsAspect(),
	m_actorAspect(m_physicsAspect),
	m_renderAspect(nullptr),
	m_entityAspect(),
	m_bRun(0),
	m_fileAspect(),
	m_bRunFileThread(),
	m_fileAspectThread(),
	m_scene(),
	m_renderAspectDll(nullptr)
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
	// process events
	m_eventManager.update();

	// update aspects in order

	m_physicsAspect.fetch();

	//////////////
	//m_profiler.pushCpuMarker("update.input()");

	m_systemAspect.update(g_dt);

	m_entityAspect.updateInput(g_dt);

	//m_profiler.popCpuMarker();
	//////////////

	//////////////

	//CpuProfiler::pushMarker("actor");
	m_actorAspect.update();
	//CpuProfiler::popMarker();

	//////////////

	//CpuProfiler::pushMarker("physics");
	m_entityAspect.updatePhysics_linear(g_dt);
	//CpuProfiler::popMarker();

	//CpuProfiler::pushMarker("actor position");
	m_entityAspect.updatePlayerPositionCamera();
	m_entityAspect.updateActorTransforms();
	//CpuProfiler::popMarker();

	//CpuProfiler::pushMarker("physx");
	m_physicsAspect.update(g_dt);
	//CpuProfiler::popMarker();
}

void Engine::renderLoop()
{
	m_renderAspect->makeCurrent(true);
	if (!m_renderAspect->initializeProfiler())
	{
		VX_ASSERT(false);
		return;
	}

	auto frequency = Timer::getFrequency();
	const f64 invFrequency = 1.0 / frequency;

	//CpuProfiler::setPosition(vx::float2(0, 500));

	//CpuProfiler::initialize(&m_renderAspect.getProfilerFont());

	//Video video;
	//video.initialize("test.video");

	LARGE_INTEGER last;
	QueryPerformanceCounter(&last);

	f32 accum = 0.0f;
	while (m_bRunRenderThread.load() != 0)
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		auto frameTicks = (current.QuadPart - last.QuadPart) * 1000;
		f32 frameTime = frameTicks * invFrequency * 0.001f;
		frameTime = fminf(frameTime, g_dt);

		accum += frameTime;

		//CpuProfiler::frame();

		//CpuProfiler::pushMarker("frame"); // frame begin

		while (accum >= g_dt)
		{
			m_renderAspect->updateProfiler(g_dt);
			//CpuProfiler::update();
			//CpuProfiler::updateRenderer();

			accum -= g_dt;
		}

		//CpuProfiler::pushMarker("update");
		m_renderAspect->update();
		//CpuProfiler::popMarker();

		//CpuProfiler::pushMarker("render");
		m_renderAspect->render();
		//CpuProfiler::popMarker();

		//CpuProfiler::popMarker(); // frame end

		last = current;
	}

	//video.shutdown();
}

void Engine::mainLoop()
{
	auto frequency = Timer::getFrequency();
	const f64 invFrequency = 1.0 / frequency;

	LARGE_INTEGER last;
	QueryPerformanceCounter(&last);

	auto &font = m_renderAspect->getProfilerFont();
	//CpuProfiler::initialize(&font);

	f32 accum = 0.0f;
	while (m_bRun != 0)
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		auto frameTicks = (current.QuadPart - last.QuadPart) * 1000;
		f32 frameTime = frameTicks * invFrequency * 0.001f;
		frameTime = fminf(frameTime, g_dt);

		accum += frameTime;

		//CpuProfiler::frame();
		//m_profileGraph.frame(frameTime);
		//m_profiler.frame();

		//m_profileGraph.startCpu();
		////////////// FRAME START
		//m_profiler.pushCpuMarker("frame()");

		while (accum >= g_dt)
		{
			//CpuProfiler::pushMarker("update");
			//m_profiler.pushCpuMarker("update()");
			update();
			//m_profiler.popCpuMarker();

			accum -= g_dt;

			//CpuProfiler::popMarker();

			//CpuProfiler::update();

			//	m_profiler.update(g_dt);
			//m_profileGraph.update();
		}

		//////////////
		//m_profiler.pushCpuMarker("render()");
		//m_renderAspect.render();
		//m_profiler.popCpuMarker();
		//////////////

		////////////// FRAME END
		//m_profiler.popCpuMarker();
		//m_profileGraph.endCpu();

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

	m_eventManager.initialize(&m_allocator, 255);
	Locator::provide(&m_eventManager);

	if (!m_fileAspect.initialize(&m_allocator, dataDir, &m_eventManager, nullptr))
		return false;

	return true;
}

bool Engine::createRenderAspectGL(const std::string &dataDir, const RenderAspectDescription &desc)
{
	auto handle = LoadLibrary(L"../../lib/vxRenderAspectGL_d.dll");
	if (handle == nullptr)
		return false;

	auto proc = (CreateRenderAspectFunction)GetProcAddress(handle, "createRenderAspect");
	if (proc == nullptr)
		return false;

	auto renderAspect = proc(dataDir, desc, &g_engineConfig, &m_fileAspect, &m_eventManager);
	if (renderAspect == nullptr)
		return false;

	m_renderAspect = renderAspect;
	m_renderAspectDll = handle;

	//if (!m_renderAspect->initialize(dataDir, desc, &g_engineConfig, &m_fileAspect, &m_eventManager))
	//	return false;

	return true;
}

bool Engine::initialize()
{
	const std::string dataDir("../data/");

	if (!g_engineConfig.loadFromFile("settings.txt"))
	{
		printf("could not load 'settings.txt'\n");
		return false;
	}

	if (!initializeImpl(dataDir))
		return false;

	if (!m_systemAspect.initialize(g_engineConfig, ::callbackKeyPressed, nullptr))
		return false;

	RenderAspectDescription renderAspectDesc =
	{
		&m_systemAspect.getWindow() ,
		&m_allocator,
		g_engineConfig.m_resolution,
		vx::degToRad(g_engineConfig.m_fov),
		g_engineConfig.m_zNear,
		g_engineConfig.m_zFar,
		g_engineConfig.m_vsync,
		g_engineConfig.m_renderDebug
	};


	if (!createRenderAspectGL(dataDir, renderAspectDesc))
	{
		return false;
	}

	m_renderAspect->makeCurrent(false);

	if (!m_physicsAspect.initialize())
		return false;

	if (!m_entityAspect.initialize(&m_allocator))
		return false;

#if _VX_AUDIO
	if (!m_audioAspect.initialize())
		return false;
#endif

	m_actorAspect.initialize(m_entityAspect, &m_allocator);

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
	m_renderThread.join();

	m_scene.reset();

#if _VX_AUDIO
	m_audioAspect.shutdown();
#endif
	m_entityAspect.shutdown();
	m_physicsAspect.shutdown();
	m_renderAspect->shutdown(m_systemAspect.getWindow().getHwnd());
	m_systemAspect.shutdown();
	m_fileAspect.shutdown();
	m_shutdown = 1;

	Locator::reset();

	//CpuProfiler::shutdown();

	m_allocator.release();

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
	m_renderThread = vx::thread(&Engine::renderLoop, this);

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
	if (key == vx::Keyboard::Key_Escape)
	{
		stop();
	}
	/*else if (key == vx::Keyboard::Key_Num0)
	{
	dev::g_showNavGraph = dev::g_showNavGraph ^ 1;
	}
	else if(key == vx::Keyboard::Key_Num1)
	{
	dev::g_toggleRender = dev::g_toggleRender ^ 1;
	}*/
	else
	{
		m_renderAspect->keyPressed(key);
	}
}