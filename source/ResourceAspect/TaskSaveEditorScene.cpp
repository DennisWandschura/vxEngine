#include "TaskSaveEditorScene.h"
#include <vxLib/File/File.h>
#include <vxResourceAspect/SceneFactory.h>

TaskSaveEditorScene::TaskSaveEditorScene(TaskSaveEditorSceneDesc &&desc)
:Task(std::move(desc.m_evt)),
m_fileNameWithPath(std::move(desc.m_fileNameWithPath)),
m_scene(desc.m_scene)
{

}

TaskSaveEditorScene::~TaskSaveEditorScene()
{

}

TaskReturnType TaskSaveEditorScene::runImpl()
{
	vx::File f;
	if (!f.create(m_fileNameWithPath.c_str(), vx::FileAccess::Write))
	{
		return TaskReturnType::Failure;
	}

	SceneFactory::saveToFile(*m_scene, &f);
	SceneFactory::deleteScene(m_scene);
	m_scene = nullptr;

	return TaskReturnType::Success;
}

f32 TaskSaveEditorScene::getTimeMs() const
{
	return 0.0f;
}