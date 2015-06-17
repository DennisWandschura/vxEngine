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
#include <vxEngineLib/Logfile.h>
#include <vxEngineLib/Timer.h>
#include <csignal>
#include <vxEngineLib/Scene.h>
#include <vxEngineLib/debugPrint.h>

#include "SmallObjAllocator.h"
#include "SmallObject.h"

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
			g_engine->stop();
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

vx::float2 encodeNormal(const vx::float3 &n)
{
	vx::float2 enc;
	enc.x = atan2(n.y, n.x) / vx::VX_PI;
	enc.y = n.z;

	return (enc + 1.0) * 0.5;
}

vx::float3 decodeNormal(const vx::float2 &enc)
{
	vx::float3 n;

	vx::float2 ang = enc * 2.0f - 1.0f;

	float tmp = ang.x * vx::VX_PI;

	vx::float2 scth;
	scth.x = sin(tmp);
	scth.y = cos(tmp);

	vx::float2 scphi;
	scphi.x = (1.0 - ang.y*ang.y);
	scphi.y = ang.y;

	n.x = scth.y * scphi.x;
	n.y = scth.x * scphi.x;
	n.z = scphi.y;

	return n;
}

int main()
{
	auto previousHandler = std::signal(SIGABRT, signalHandler);
	if (previousHandler == SIG_ERR)
	{
		return 1;
	}

	const __m128 qRotations_normal[6] =
	{
		{ 0.382683426, 0.000000000, 0.000000000, 0.923879504 },
		{ -0.382683426, 0.000000000, 0.000000000, 0.923879504 },
		{ 0.000000000, 0.382683426, 0.000000000, 0.923879504 },
		{ 0.000000000, -0.382683426, 0.000000000, 0.923879504 },
		{ 0.000000000, 0.000000000, 0.382683426, 0.923879504 },
		{ 0.000000000, 0.000000000, -0.382683426, 0.923879504 }
	};

	__m128 normal = {1, 0, 0, 0};

	const int kll = 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 4 | 1 << 5 | 1 << 6 | 1 << 7 |
		1 << 8 | 1 << 9 | 1 << 10 | 1 << 11 | 1 << 12 | 1 << 13 | 1 << 14;

	auto n1 = vx::quaternionRotation(normal, qRotations_normal[0]);
	auto n2 = vx::quaternionRotation(normal, qRotations_normal[1]);
	auto n3 = vx::quaternionRotation(normal, qRotations_normal[2]);

	auto n4 = vx::quaternionRotation(normal, qRotations_normal[3]);
	auto n5 = vx::quaternionRotation(normal, qRotations_normal[4]);
	auto n6 = vx::quaternionRotation(normal, qRotations_normal[5]);

	const auto mask = 0xffffffff;

	const auto fil = 1 << 1;
	const auto resl = mask & fil;

	HANDLE hLogFile = nullptr;
	hLogFile = CreateFileA("log.txt", GENERIC_WRITE,
		FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_ERROR, hLogFile);

	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, hLogFile);

	SmallObjAllocator alloc(1 KBYTE);
	SmallObject::setAllocator(&alloc);

	Timer mainTimer;
	Logfile mainLogfile(mainTimer);
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
	vx::activateChannel(vx::debugPrint::Channel_FileAspect);

	SCOPE_EXIT
	{
		scene.reset();
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

	engine.requestLoadFile(vx::FileEntry("test13.scene", vx::FileType::Scene), &scene);

	LOG(mainLogfile, "Starting", false);

	engine.start();

	std::atomic_int sizeFront;
	int sizeBack = 0;

	std::atomic<void*> ptr;
	void* ptrBack;

	__cplusplus;

	return 0;
}
#endif