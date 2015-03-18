#include "Engine.h"
#include "Clock.h"
#include "yamlHelper.h"
#include "Locator.h"
#include "developer.h"

const U32 g_hz = 30u;
const F32 g_dt = 1.0f / g_hz;

Engine* g_pEngine{nullptr};

namespace
{
	void callbackKeyPressed(U16 key)
	{
		g_pEngine->keyPressed(key);
	}

	void handleInput(const vx::Mouse &m, const vx::Keyboard &k, F32 dt)
	{
		g_pEngine->handleInput(m, k, dt);
	}
}

Engine::Engine(Logfile &logfile)
	:m_eventManager(),
	m_systemAspect(),
	m_physicsAspect(m_fileAspect),
	m_actorAspect(m_physicsAspect),
	m_renderAspect(logfile, m_fileAspect),
	m_entityAspect(m_physicsAspect, m_renderAspect.getCamera(), m_fileAspect, m_renderAspect),
	m_bRun(0),
	m_fileAspect(m_eventManager),
	m_bRunFileThread(),
	m_fileAspectThread()
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

void Engine::mainLoop()
{
	auto frequency = Clock::getFrequency();
	const F64 invFrequency = 1.0 / frequency;

	LARGE_INTEGER last;
	QueryPerformanceCounter(&last);

	const U32 bufferSize = 4096 * 8;
	U8 musicBuffer[bufferSize];
	//ogg_stream music;
	///assert(music.open("data/music/Complex Terms.ogg"));
	//music.setVolume(0.2f);
	//music.playback(musicBuffer, bufferSize);
	//music.play(musicBuffer, bufferSize);

	F32 accum = 0.0f;
	while (m_bRun != 0)
	{
		LARGE_INTEGER current;
		QueryPerformanceCounter(&current);

		auto frameTicks = (current.QuadPart - last.QuadPart) * 1000;
		F32 frameTime = frameTicks * invFrequency * 0.001f;
		frameTime = fminf(frameTime, g_dt);

		accum += frameTime;

		m_profileGraph.frame(frameTime);
		m_profiler.frame();

		m_profileGraph.startCpu();
		////////////// FRAME START
		m_profiler.pushCpuMarker("frame()");

		while (accum >= g_dt)
		{
			//if (music.isPlaying())
			//music.update(musicBuffer, bufferSize);
			////////////// UPDATE START
			m_profiler.pushCpuMarker("update()");

			// process events
			m_eventManager.update();

			// update aspects in order

			m_physicsAspect.fetch();

			//////////////
			m_profiler.pushCpuMarker("update.input()");

			m_systemAspect.update(g_dt);

			m_entityAspect.updateInput(g_dt);

			m_profiler.popCpuMarker();
			//////////////

			//////////////

			m_actorAspect.update(g_dt);

			//////////////

			//////////////
			m_profiler.pushCpuMarker("update.physics()");

			m_entityAspect.updatePhysics_linear(g_dt);

			m_physicsAspect.update(g_dt);

			m_profiler.popCpuMarker();
			//////////////

			//////////////
			m_profiler.pushCpuMarker("update.position()");

			m_entityAspect.updatePlayerPositionCamera();
			m_entityAspect.updateActorTransforms();

			m_profiler.popCpuMarker();
			//////////////

			//////////////
			m_profiler.pushCpuMarker("update.render()");

			m_renderAspect.update();

			m_profiler.popCpuMarker();
			//////////////

			////////////// UPDATE END
			m_profiler.popCpuMarker();

			accum -= g_dt;

			m_profiler.update(g_dt);
			m_profileGraph.update();
		}

		//////////////
		m_profiler.pushCpuMarker("render()");
		m_renderAspect.render(&m_profiler, &m_profileGraph);
		m_profiler.popCpuMarker();
		//////////////

		////////////// FRAME END
		m_profiler.popCpuMarker();
		m_profileGraph.endCpu();

		last = current;
	}

	//music.stop();
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

	if (!m_fileAspect.initialize(&m_allocator, dataDir))
		return false;

	return true;
}

bool Engine::initialize()
{
	const std::string dataDir("data/");

	if (!initializeImpl(dataDir))
		return false;

	YAML::Node settingsFile = YAML::LoadFile("settings.yaml");
	auto resNode = settingsFile["settings"];
	auto windowResolution = resNode["window_resolution"].as<vx::uint2>();
	F32 fov = resNode["fov"].as<F32>();
	F32 z_near = resNode["z_near"].as<F32>();
	F32 z_far = resNode["z_far"].as<F32>();
	bool vsync = resNode["vsync"].as<bool>();
	bool debug = resNode["debug"].as<bool>();

	if (!m_systemAspect.initialize(windowResolution, ::callbackKeyPressed, ::handleInput))
		return false;

	RenderAspectDesc renderAspectDesc;
	renderAspectDesc.window = &m_systemAspect.getWindow();
	renderAspectDesc.resolution = windowResolution;
	renderAspectDesc.fovRad = vx::degToRad(fov);
	renderAspectDesc.z_near = z_near;
	renderAspectDesc.z_far = z_far;
	renderAspectDesc.vsync = vsync;
	renderAspectDesc.debug = debug;
	renderAspectDesc.targetMs = g_dt;
	renderAspectDesc.pAllocator=&m_allocator;
	renderAspectDesc.pProfiler=&m_profiler;
	renderAspectDesc.pGraph=&m_profileGraph;

	if (!m_renderAspect.initialize(dataDir, renderAspectDesc))
		return false;

	if (!m_physicsAspect.initialize())
		return false;

	if (!m_entityAspect.initialize(&m_allocator))
		return false;

	m_actorAspect.initialize(m_entityAspect, m_eventManager, &m_allocator);

	Locator::provide(&m_physicsAspect);

	// register aspects that receive events
	m_eventManager.registerListener(&m_renderAspect, 3);
	m_eventManager.registerListener(&m_physicsAspect, 2);
	m_eventManager.registerListener(&m_entityAspect, 1);
	m_eventManager.registerListener(&m_actorAspect, 2);

	m_bRun = 1;
	m_bRunFileThread.store(1);
	m_shutdown = 0;

	Locator::provide(&m_eventManager);

	return true;
}

void Engine::shutdown()
{
	m_fileAspectThread.join();

	m_entityAspect.shutdown();
	m_physicsAspect.shutdown();
	m_renderAspect.shutdown(m_systemAspect.getWindow().getHwnd());
	m_systemAspect.shutdown();
	m_fileAspect.shutdown();
	m_shutdown = 1;

	m_allocator.release();
}

void Engine::start()
{
	m_fileAspectThread = vx::thread(&Engine::loopFileThread, this);
	mainLoop();
}

void Engine::stop()
{
	m_bRun = 0;
	m_bRunFileThread.store(0);
}

void Engine::requestLoadFile(const FileEntry &fileEntry, void* p)
{
	m_fileAspect.requestLoadFile(fileEntry, p);
}

void Engine::keyPressed(U16 key)
{
	if (key == VK_ESCAPE)
	{
		stop();
	}

	if (key == VK_NUMPAD0)
	{
		dev::g_showNavGraph = dev::g_showNavGraph ^ 1;
	}

	if (key == VK_NUMPAD1)
	{
		dev::g_toggleRender = dev::g_toggleRender ^ 1;
	}

	m_renderAspect.keyPressed(key);
	m_entityAspect.keyPressed(key);
}

void Engine::handleInput(const vx::Mouse &m, const vx::Keyboard &k, F32 dt)
{
	m_entityAspect.handleKeyboard(k);
	m_entityAspect.handleMouse(m, dt);
}