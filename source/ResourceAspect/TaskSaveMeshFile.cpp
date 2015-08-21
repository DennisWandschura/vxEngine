#include "TaskSaveMeshFile.h"
#include <vxEngineLib/FileFactory.h>
#include <vxEngineLib/MeshFile.h>

TaskSaveMeshFile::TaskSaveMeshFile(std::string &&fileNameWithPath, const vx::MeshFile* meshFile)
	:m_fileNameWithPath(std::move(fileNameWithPath)),
	m_meshFile(meshFile)
{

}

TaskSaveMeshFile::~TaskSaveMeshFile()
{

}

TaskReturnType TaskSaveMeshFile::runImpl()
{
	vx::FileFactory::saveToFile(m_fileNameWithPath.c_str(), m_meshFile);

	return TaskReturnType::Success;
}

f32 TaskSaveMeshFile::getTimeMs() const
{
	return 0.0f;
}