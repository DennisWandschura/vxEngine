#pragma once

namespace vx
{
	class MeshFile;
}

#include <vxEngineLib/Task.h>
#include <string>

class TaskSaveMeshFile : public Task
{
	std::string m_fileNameWithPath;
	const vx::MeshFile* m_meshFile;

	TaskReturnType runImpl() override;

public:
	TaskSaveMeshFile(std::string &&fileNameWithPath, const vx::MeshFile* meshFile);
	~TaskSaveMeshFile();

	f32 getTimeMs() const override;
};