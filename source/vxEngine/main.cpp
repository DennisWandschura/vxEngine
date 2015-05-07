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
#include "libraries.h"
#if _VX_EDITOR
#else
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include <vxLib/ScopeGuard.h>
#include "Engine.h"
#include "Logfile.h"
#include "Clock.h"
#include "enums.h"
#include <csignal>

#include "Scene.h"
#include "File.h"

#include "SmallObjAllocator.h"
#include "SmallObject.h"
#include "SceneFile.h"

namespace
{
	Logfile* g_logfile{ nullptr };
	Engine* g_engine{ nullptr };

	void shutdown()
	{
		if (g_logfile)
		{
			g_logfile->close();
			g_logfile = nullptr;
		}

		if (g_engine)
		{
			g_engine->shutdown();
			g_engine = nullptr;
		}
	}

	void signalHandler(int signal)
	{
		if (signal == SIGABRT)
		{
			puts("SIGABRT received");
			shutdown();
		}
		else
		{
			printf("Unexpected signal %d  received\n", signal);
		}
		std::exit(EXIT_FAILURE);
	}
}

int main()
{
	auto previousHandler = std::signal(SIGABRT, signalHandler);
	if (previousHandler == SIG_ERR)
	{
		return 1;
	}

	HANDLE hLogFile = nullptr;;
	hLogFile = CreateFileA("log.txt", GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, hLogFile);

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, hLogFile);

	SmallObjAllocator alloc(1 KBYTE);
	SmallObject::setAllocator(&alloc);

	Clock mainClock;
	Logfile mainLogfile(mainClock);
	g_logfile = &mainLogfile;

	if (!mainLogfile.create("logfile.xml"))
	{
		g_logfile = nullptr;
		return 1;
	}

	_CrtMemState state;
	// create a checkpoint to for current memory state
	_CrtMemCheckpoint(&state);

	Scene scene;
	Engine engine;

	g_engine = &engine;
	{
		//SceneFile sceneFile;
		//sceneFile.loadFromYAML("data/scenes/scene5.scene.yaml");
	//	sceneFile.saveToFile("data/scenes/scene5.scene");
	}

	SCOPE_EXIT
	{
		engine.shutdown();
		LOG(mainLogfile, "Shutting down Engine", false);
		mainLogfile.close();

		_CrtMemDumpAllObjectsSince(&state);

		CloseHandle(hLogFile);

		g_logfile = nullptr;
	};

	LOG(mainLogfile, "Initializing Engine", false);
	if (!engine.initialize())
	{
		LOG_ERROR(mainLogfile, "Error initializing Engine !", false);
		return 1;
	}

	engine.requestLoadFile(FileEntry("test1.scene", FileType::Scene), &scene);

	LOG(mainLogfile, "Starting", false);

	engine.start();

	return 0;
}
#endif