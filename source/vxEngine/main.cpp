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
		SceneFile sceneFile;
		sceneFile.loadFromYAML("data/scenes/scene5.scene.yaml");
		sceneFile.saveToFile("data/scenes/scene5.scene");
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

	engine.requestLoadFile(FileEntry("scene5.scene", FileType::Scene), &scene);

	LOG(mainLogfile, "Starting", false);

	engine.start();

	Locator::reset();

	return 0;
}
#endif