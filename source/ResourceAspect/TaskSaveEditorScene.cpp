#include "TaskSaveEditorScene.h"
#include <vxLib/File/File.h>
#include <vxResourceAspect/SceneFactory.h>
#include <vxLib/ScopeGuard.h>

TaskSaveEditorScene::TaskSaveEditorScene(TaskSaveEditorSceneDesc &&desc)
:Task(std::move(desc.m_evt)),
m_fileNameWithPath(std::move(desc.m_fileNameWithPath)),
m_scene(desc.m_scene),
m_actorResManager(desc.m_actorResManager)
{

}

TaskSaveEditorScene::~TaskSaveEditorScene()
{

}

TaskReturnType TaskSaveEditorScene::runImpl()
{
	SCOPE_EXIT
	{
		SceneFactory::deleteScene(m_scene);
	m_scene = nullptr;
	};

	if(!SceneFactory::saveToFile(*m_scene, m_fileNameWithPath.c_str(), m_actorResManager))
	{
		return TaskReturnType::Failure;
	}

	return TaskReturnType::Success;
}

f32 TaskSaveEditorScene::getTimeMs() const
{
	return 0.0f;
}