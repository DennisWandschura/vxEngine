#pragma once

class ActorFile;

#include <vxEngineLib/Task.h>
#include <string>

class TaskSaveActorFile : public Task
{
	std::string m_fileNameWithPath;
	const ActorFile* m_actorFile;

	TaskReturnType runImpl() override;

public:
	TaskSaveActorFile(std::string &&fileNameWithPath, const ActorFile* actorFile);
	~TaskSaveActorFile();

	f32 getTimeMs() const override;
};