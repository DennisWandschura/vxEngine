#pragma once

class ResourceAspect;

template<typename T>
class ResourceManager;

namespace physx
{
	class PxCooking;
}

namespace vx
{
	class MeshFile;
}

#include <vxEngineLib/Task.h>
#include <vxEngineLib/PhysxEnums.h>
#include <string>
#include <FbxFactoryInterface.h>

struct TaskLoadFbxDesc
{
	Event m_event;
	ResourceAspect* m_resourceAspect;
	void* m_userData;
	ResourceManager<vx::MeshFile>* m_meshManager;
	physx::PxCooking* m_cooking;
	std::string m_fileNameWithPath;
	std::string m_meshFolder;
	std::string m_animationFolder;
	PhsyxMeshType m_physxMeshType;
	FbxFactoryInterface* m_fbxFactory;
};

class TaskLoadFbx: public Task
{
	ResourceAspect* m_resourceAspect;
	void* m_userData;
	ResourceManager<vx::MeshFile>* m_meshManager;
	physx::PxCooking* m_cooking;
	std::string m_fileNameWithPath;
	std::string m_meshFolder;
	std::string m_animationFolder;
	PhsyxMeshType m_physxMeshType;
	FbxFactoryInterface* m_fbxFactory;

	TaskReturnType runImpl() override;

public:
	TaskLoadFbx(TaskLoadFbxDesc &&rhs);
	~TaskLoadFbx();

	f32 getTimeMs() const override;

	const char* getName(u32* size) const override
	{
		*size = 16;
		return "TaskLoadFbxDesc";
	}
};