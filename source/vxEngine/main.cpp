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
#include <vxLib/ScopeGuard.h>
#include "Engine.h"
#include "Logfile.h"
#include "Clock.h"
#include "enums.h"
#include "Locator.h"

#include "Scene.h"
#include "File.h"

#include "SmallObjAllocator.h"
#include "SmallObject.h"
#include "SceneFile.h"

int main()
{
	SmallObjAllocator alloc(1 KBYTE);
	SmallObject::setAllocator(&alloc);

	Clock mainClock;
	Logfile mainLogfile(mainClock);

	mainLogfile.create("logfile.xml");
	SCOPE_EXIT
	{
		LOG(mainLogfile, "Shutting down Engine", false);
		mainLogfile.close();
	};

	Scene scene;
	Engine engine;

	{
		//SceneFile sceneFile;
		//sceneFile.loadFromYAML("data/scenes/scene5.scene.yaml");
	//	sceneFile.saveToFile("data/scenes/scene5.scene");
	}

	SCOPE_EXIT
	{
		engine.shutdown();
	};

	LOG(mainLogfile, "Initializing Engine", false);
	if (!engine.initialize())
	{
		LOG_ERROR(mainLogfile, "Error initializing Engine !", false);
		return 1;
	}

	engine.requestLoadFile(FileEntry("test.scene", FileType::Scene), &scene);

	LOG(mainLogfile, "Starting", false);

	engine.start();

	Locator::reset();

	return 0;
}
#endif