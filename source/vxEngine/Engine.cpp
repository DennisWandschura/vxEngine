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
#include <vxEngineLib/EngineConfig.h>
#include <vxEngineLib/Locator.h>
#include "developer.h"
#include "EngineGlobals.h"
#include <vxEngineLib/MessageTypes.h>
#include <vxEngineLib/debugPrint.h>
#include <vxEngineLib/FileEntry.h>
#include <vxEngineLib/Logfile.h>

Engine* g_pEngine{ nullptr };

namespace EngineCpp
{
	void callbackKeyPressed(u16 key)
	{
		g_pEngine->keyPressed(key);
	}

	void callbackKeyReleased(u16 key)
	{
		g_pEngine->keyReleased(key);
	}

	void schedulerThread(vx::TaskManager* scheduler)
	{
		std::atomic_uint running;
		running.store(1);

		scheduler->initializeThread(&running);

		while (running.load() != 0)
		{
			scheduler->swapBuffer();

			scheduler->update();
		}
	}

	const wchar_t* g_dx12DllDebug = L"../../lib/vxRenderAspectDX12_d.dll";
	const wchar_t* g_dx12Dll = L"../../lib/vxRenderAspectDX12.dll";
	const wchar_t* g_glDllDebug = L"../../lib/vxRenderAspectGL_d.dll";
	const wchar_t* g_glDll = L"../../lib/vxRenderAspectGL.dll";
}

Engine::Engine()
	:m_msgManager(),
	m_actionManager(),
	m_taskManager(),
	m_systemAspect(),
	m_physicsAspect(),
	m_actorAspect(),
	m_threadHandle(nullptr),
	m_lastTime(0),
	m_lastSystem(0),
	m_renderAspect(nullptr),
	m_entityAspect(),
	m_bRun(0),
	m_resourceAspect(),
#if _VX_MEM_PROFILE
	m_allocManager(),
#endif
	m_allocator(),
	m_shutdown(0),
	m_scene(),
	m_renderAspectDll(nullptr),
	m_audioAspectDll(nullptr)
{
	g_pEngine = this;
}

Engine::~Engine()
{
	if (m_shutdown == 0)
	{
		// something bad happened
		VX_ASSERT(false);
	}
}

void Engine::update()
{
	m_physicsAspect.update(g_dt);

	// process events
	m_cpuProfiler.pushMarker("update msg");
	m_msgManager.update();
	m_cpuProfiler.popMarker();

	m_cpuProfiler.pushMarker("update action");
	m_actionManager.update();
	m_cpuProfiler.popMarker();

	m_resourceAspect.update();

	m_cpuProfiler.pushMarker("update audio()");
	m_audioAspect->update(g_dt, m_entityAspect.getPlayerPosition());
	m_cpuProfiler.popMarker();

	// update aspects in order
	m_cpuProfiler.pushMarker("physx wait");
	m_physicsAspect.fetch();
	m_cpuProfiler.popMarker();

	m_systemAspect.update(g_dt);

	m_entityAspect.update(g_dt, &m_actionManager);

	//getThreadInfo();
}

void Engine::getThreadInfo()
{
	u64 creationTime, exitTime, kernelTime, userTime;
	auto hr = GetThreadTimes(m_threadHandle, (FILETIME*)&creationTime, (FILETIME*)&exitTime, (FILETIME*)&kernelTime, (FILETIME*)&userTime);

	u64 threadTime = kernelTime + userTime;

	FILETIME systemIdle;
	u64 systemKernel, systemUser;
	GetSystemTimes(&systemIdle, (FILETIME*)&systemKernel, (FILETIME*)&systemUser);
	u64 systemTime = systemKernel + systemUser;

	u64 diffThread = threadTime - m_lastTime;
	u64 diffSystem = systemTime - m_lastSystem;

	auto cpuUsage = (100.0 * diffThread) / diffSystem;
	//printf("%f\n", cpuUsage);

	m_lastTime = threadTime;
	m_lastSystem = systemTime;
}

void Engine::mainLoop(Logfile* logfile)
{
	m_renderAspect->initializeProfiler(logfile);

	CpuTimer timer;

	f32 accum = 0.0f;
	while (m_bRun != 0)
	{
		auto frameTime = timer.getTimeSeconds();
		timer.reset();

		frameTime = fminf(frameTime, g_dt);
		accum += frameTime;

		m_cpuProfiler.frame();

		m_cpuProfiler.pushMarker("frame");

		m_cpuProfiler.pushMarker("build commands");
		m_renderAspect->buildCommands();
		m_cpuProfiler.popMarker();

		m_cpuProfiler.pushMarker("submit commands");
		m_renderAspect->submitCommands();
		m_cpuProfiler.popMarker();

		m_taskManager.updateMainThread();

		while (accum >= g_dt)
		{
			m_cpuProfiler.pushMarker("update()");
			update();

			m_cpuProfiler.pushMarker("update renderer");
			m_renderAspect->update();
			m_cpuProfiler.popMarker();

			m_cpuProfiler.update(m_renderAspect);
			m_renderAspect->updateProfiler(g_dt);

			accum -= g_dt;
			m_cpuProfiler.popMarker();
		}

		m_cpuProfiler.pushMarker("gpu wait");
		m_renderAspect->wait();
		m_cpuProfiler.popMarker();

		m_cpuProfiler.pushMarker("swap buffers");
		m_renderAspect->swapBuffers();
		m_cpuProfiler.popMarker();

		m_cpuProfiler.popMarker();
	}
}

bool Engine::initializeImpl(const std::string &dataDir)
{
	m_memory = Memory(g_totalMemory, 64);

	m_allocator = vx::StackAllocator(m_memory.get(), static_cast<u32>(m_memory.size()));

#if _VX_MEM_PROFILE
	m_allocManager.registerAllocator(&m_allocator, "mainAllocator");
#endif

	m_msgManager.initialize(&m_allocator, 255);
	Locator::provide(&m_msgManager);

	bool flipTextures = (g_engineConfig.m_rendererSettings.m_renderMode == Graphics::RendererSettings::Mode_GL);
	if (!m_resourceAspect.initialize(&m_allocator, dataDir, nullptr, &m_taskManager, &m_msgManager, flipTextures, false))
		return false;

	return true;
}

bool Engine::createRenderAspect(Graphics::RendererSettings::Mode mode)
{

	const wchar_t* dllFile = nullptr;

	switch (mode)
	{
	case Graphics::RendererSettings::Mode_GL:
	{
#if _DEBUG
		dllFile = EngineCpp::g_glDllDebug;
#else
		dllFile = EngineCpp::g_glDll;
#endif
	}break;
	case Graphics::RendererSettings::Mode_DX12:
	{
#if _DEBUG
		dllFile = EngineCpp::g_dx12DllDebug;
#else
		dllFile = EngineCpp::g_dx12Dll;
#endif
	}break;
	default:
		break;
	}

	auto handle = LoadLibrary(dllFile);
	if (handle == nullptr)
		return false;

	auto proc = (CreateRenderAspectFunction)GetProcAddress(handle, "createRenderAspect");
	auto procDestroy = (DestroyRenderAspectFunction)GetProcAddress(handle, "destroyRenderAspect");
	if (proc == nullptr || procDestroy == nullptr)
		return false;

	auto renderAspect = proc();
	if (renderAspect == nullptr)
		return false;

	m_renderAspect = renderAspect;
	m_renderAspectDll = handle;

	return true;
}

void Engine::destroyRenderAspect()
{
	auto procDestroy = (DestroyRenderAspectFunction)GetProcAddress(m_renderAspectDll, "destroyRenderAspect");
	procDestroy(m_renderAspect);
	m_renderAspect = nullptr;
}

bool Engine::createAudioAspect()
{
#if _DEBUG
	auto handle = LoadLibrary(L"../../lib/vxAudioAspect_d.dll");
#else
	auto handle = LoadLibrary(L"../../lib/vxAudioAspect.dll");
#endif
	if (handle == nullptr)
		return false;

	auto proc = (CreateAudioAspectFunctionType)GetProcAddress(handle, "createAudioAspect");
	if (proc == nullptr)
		return false;

	auto audioAspect = proc();
	if (audioAspect == nullptr)
		return false;

	m_audioAspect = audioAspect;
	m_audioAspectDll = handle;

	if (!m_audioAspect->initialize(&m_resourceAspect))
		return false;

	return true;
}

bool Engine::initialize(Logfile* logfile, SmallObjAllocator* smallObjAllocatorMainThread, AbortSignalHandlerFun signalHandlerFn)
{
	const std::string dataDir("../data/");

	m_threadHandle = GetCurrentThread();

#if _VX_MEM_PROFILE
	vx::Allocator::setProfiler(&m_allocManager);
#endif

	if (!g_engineConfig.loadFromFile("settings.txt"))
	{
		logfile->append("could not load 'settings.txt'\n");
		//printf("could not load 'settings.txt'\n");
		return false;
	}

	auto topLeftProfilerPosition = (g_engineConfig.m_resolution / vx::uint2(2)) - vx::uint2(300, 30);
	m_cpuProfiler.initialize(topLeftProfilerPosition);

	g_engineConfig.m_editor = false;

	if (!initializeImpl(dataDir))
		return false;

	m_taskManager.initialize(2, 20, 30.0f, &m_allocator);

	if (!m_systemAspect.initialize(g_engineConfig, EngineCpp::callbackKeyPressed, EngineCpp::callbackKeyReleased, nullptr))
		return false;

	RenderAspectDescription renderAspectDesc =
	{
		dataDir,
		(void*)&m_systemAspect.getWindow(),
		nullptr,
		&m_allocator,
		&g_engineConfig,
		logfile,
		&m_cpuProfiler,
		&m_resourceAspect,
		&m_msgManager,
		&m_taskManager,
		smallObjAllocatorMainThread
	};

	auto renderMode = g_engineConfig.m_rendererSettings.m_renderMode;
	if (!createRenderAspect(renderMode))
	{
		return false;
	}

	if (m_renderAspect->initialize(renderAspectDesc, signalHandlerFn) != RenderAspectInitializeError::OK)
	{
		logfile->append("error initializing render aspect\n");
		return false;
	}

	if (!m_physicsAspect.initialize(&m_taskManager))
	{
		logfile->append("error initializing physics aspect\n");
		return false;
	}

#if _VX_MEM_PROFILE
	if (!m_entityAspect.initialize(&m_allocator, &m_taskManager, &m_allocManager, &m_resourceAspect))
		return false;
#else
	if (!m_entityAspect.initialize(&m_allocator, &m_taskManager, nullptr, &m_resourceAspect))
	{
		logfile->append("error initializing entity aspect\n");
		return false;
	}
#endif

	if (!createAudioAspect())
	{
		logfile->append("error initializing audio aspect\n");
		return false;
	}

#if _VX_MEM_PROFILE
	m_actorAspect.initialize(&m_allocator, &m_allocManager);
#else
	m_actorAspect.initialize(&m_allocator, nullptr);
#endif

	Locator::provide(&m_physicsAspect);
	Locator::provide(m_renderAspect);
	Locator::provide(&m_resourceAspect);

	// register aspects that receive events
	m_msgManager.registerListener(m_renderAspect, 3, (u8)vx::MessageType::File| (u8)vx::MessageType::Renderer);
	m_msgManager.registerListener(&m_physicsAspect, 2, (u8)vx::MessageType::File | (u8)vx::MessageType::Ingame);
	m_msgManager.registerListener(&m_entityAspect, 1, (u8)vx::MessageType::File | (u8)vx::MessageType::Ingame);
	m_msgManager.registerListener(&m_actorAspect, 2, (u8)vx::MessageType::File | (u8)vx::MessageType::Ingame | (u8)vx::MessageType::AI);
	m_msgManager.registerListener(m_audioAspect, 1, (u8)vx::MessageType::File | (u8)vx::MessageType::Audio);
		
	m_bRun = 1;
	m_shutdown = 0;

	m_taskManagerThread = std::thread(EngineCpp::schedulerThread, &m_taskManager);

	//printf("Main Thread tid: %u\n", std::this_thread::get_id());

	return true;
}

void Engine::shutdown()
{
	m_scene.reset();

	m_cpuProfiler.shutdown();

	if (m_audioAspect)
	{
		m_audioAspect->shutdown();
		delete m_audioAspect;
		m_audioAspect = nullptr;
	}
	m_entityAspect.shutdown();
	m_physicsAspect.shutdown();

	if (m_renderAspect)
	{
		m_renderAspect->shutdown(m_systemAspect.getWindow().getHwnd());
		destroyRenderAspect();
		m_renderAspect = nullptr;
	}

	m_systemAspect.shutdown();
	m_resourceAspect.shutdown();
	m_shutdown = 1;

	Locator::reset();

	m_taskManager.stop();
	if (m_taskManagerThread.joinable())
		m_taskManagerThread.join();

	m_taskManager.shutdown();

	m_allocator.clear();
	m_allocator.release();

#if _VX_MEM_PROFILE
	m_allocManager.print();
#endif

	if (m_audioAspectDll != nullptr)
	{
		FreeLibrary(m_audioAspectDll);
		m_audioAspectDll = nullptr;
	}

	if (m_renderAspectDll != nullptr)
	{
		FreeLibrary(m_renderAspectDll);
		m_renderAspectDll = nullptr;
	}

	m_memory.clear();
}

void Engine::start(Logfile* logfile)
{
	std::string level;
	g_engineConfig.m_root.get("level")->as(&level);

	vx::Variant arg;
	arg.ptr = &m_scene;
	requestLoadFile(vx::FileEntry(level.c_str(), vx::FileType::Scene), arg);

	mainLoop(logfile);
}

void Engine::stop()
{
	m_bRun = 0;
}

void Engine::requestLoadFile(const vx::FileEntry &fileEntry, vx::Variant arg)
{
	m_resourceAspect.requestLoadFile(fileEntry, arg);
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

void Engine::keyReleased(u16 key)
{
	if (key == vx::Keyboard::Key_E)
	{
		m_entityAspect.onReleasedActionKey();
	}
}