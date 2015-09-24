#include "TaskSaveActorFile.h"
#include <vxEngineLib/FileFactory.h>
#include <vxEngineLib/ActorFile.h>

TaskSaveActorFile::TaskSaveActorFile(std::string &&fileNameWithPath, const ActorFile* actorFile)
	:m_fileNameWithPath(std::move(fileNameWithPath)),
	m_actorFile(actorFile)
{

}

TaskSaveActorFile::~TaskSaveActorFile()
{

}

TaskReturnType TaskSaveActorFile::runImpl()
{
	vx::FileFactory::saveToFile(m_fileNameWithPath.c_str(), m_actorFile);

	delete(m_actorFile);

	return TaskReturnType::Success;
}

f32 TaskSaveActorFile::getTimeMs() const
{
	return 0;
}