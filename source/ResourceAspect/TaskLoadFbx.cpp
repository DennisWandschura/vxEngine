#include "TaskLoadFbx.h"
#include <vxResourceAspect/FbxFactory.h>
#include <vxResourceAspect/FileEntry.h>
#include <vxResourceAspect/ResourceAspect.h>
#include <vxResourceAspect/ResourceManager.h>

TaskLoadFbx::TaskLoadFbx(TaskLoadFbxDesc &&rhs)
	:m_resourceAspect(rhs.m_resourceAspect),
	m_userData(rhs.m_userData),
	m_meshManager(rhs.m_meshManager),
	m_cooking(rhs.m_cooking),
	m_fileNameWithPath(std::move(rhs.m_fileNameWithPath)),
	m_meshFolder(std::move(rhs.m_meshFolder)),
	m_animationFolder(std::move(rhs.m_animationFolder)),
	m_physxMeshType(rhs.m_physxMeshType)
{

}

TaskLoadFbx::~TaskLoadFbx()
{

}

TaskReturnType TaskLoadFbx::runImpl()
{
#if _VX_EDITOR
	std::vector<vx::FileHandle> meshFiles, animFiles;

	FbxFactory factory;
	if (!factory.loadFile(m_fileNameWithPath.c_str(), m_meshFolder, m_animationFolder, m_physxMeshType, m_cooking, &meshFiles, &animFiles, m_meshManager))
		return TaskReturnType::Failure;

	for (auto &it : meshFiles)
	{
		auto ptr = new std::string(it.m_string);
		vx::FileEntry fileEntry(it.m_string, vx::FileType::Mesh);

		vx::Variant arg;
		arg.ptr = ptr;
		m_resourceAspect->requestLoadFile(fileEntry, arg);
	}

	for (auto &it : animFiles)
	{
		auto ptr = new std::string(it.m_string);
		vx::FileEntry fileEntry(it.m_string, vx::FileType::Animation);

		vx::Variant arg;
		arg.ptr = ptr;
		m_resourceAspect->requestLoadFile(fileEntry, arg);
	}
#endif
	return TaskReturnType::Success;
}

f32 TaskLoadFbx::getTimeMs() const
{
	return 0.0f;
}