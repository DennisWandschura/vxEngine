#include "TaskLoadMesh.h"
#include <vxEngineLib/MeshFile.h>

TaskLoadMesh::TaskLoadMesh(shared_ptr<Event> &&evt)
	:Task(std::move(evt))
{

}

TaskLoadMesh::~TaskLoadMesh()
{

}

TaskReturnType TaskLoadMesh::createMesh(ReferenceCounted<vx::MeshFile>** ptr)
{
	u16 index;
	auto meshFilePtr = m_poolMesh->createEntry(&index, m_fileHeader.version);
	if (meshFilePtr == nullptr)
	{
		return TaskReturnType::Failure;
	}

	auto p = meshFilePtr->get().loadFromMemory(m_fileData, m_dataSize, m_allocatorMeshData);
	if (p == nullptr)
	{
		return TaskReturnType::Failure;
	}

	*ptr = meshFilePtr;

	return TaskReturnType::Success;
}

TaskReturnType TaskLoadMesh::runImpl()
{
	/*vx::lock_guard<vx::SRWMutex> lock(m_mutexLoadedFiles);

	bool result = true;
	auto it = m_sortedMeshes.find(desc.shared.sid);
	if (it == m_sortedMeshes.end())
	{
		u16 index;
		auto meshFilePtr = m_poolMesh.createEntry(&index, desc.fileHeader->version);
		VX_ASSERT(meshFilePtr != nullptr);

		//auto marker = m_allocatorMeshData.getMarker();
		auto p = meshFilePtr->get().loadFromMemory(desc.fileData, desc.size, &m_allocatorMeshData);
		if (p == nullptr)
		{
			//m_allocatorMeshData.clear(marker);
			return false;
		}

		it = m_sortedMeshes.insert(desc.shared.sid, Reference<vx::MeshFile>(*meshFilePtr));

		*desc.shared.status = vx::FileStatus::Loaded;

		LOG_ARGS(m_logfile, "Loaded Mesh '%s' %llu\n", false, desc.shared.filename, desc.shared.sid.value);
	}
	else
	{
		*desc.shared.status = vx::FileStatus::Exists;
	}*/

	std::unique_lock<std::mutex> lock(*m_mutexLoadedFiles);
	auto it = m_sortedMeshes->find(m_sid);
	lock.unlock();
	if (it != m_sortedMeshes->end())
	{
		return TaskReturnType::Success;
	}

	ReferenceCounted<vx::MeshFile>* meshFilePtr = nullptr;
	std::unique_lock<std::mutex> createMeshLock(*m_mutexCreateMesh);
	auto result = createMesh(&meshFilePtr);
	createMeshLock.unlock();

	if (result == TaskReturnType::Success)
	{
		lock.lock();
		m_sortedMeshes->insert(m_sid, Reference<vx::MeshFile>(*meshFilePtr));
		lock.unlock();
	}

	return result;
}

f32 TaskLoadMesh::getTimeMs() const
{
	return 0.0f;
}