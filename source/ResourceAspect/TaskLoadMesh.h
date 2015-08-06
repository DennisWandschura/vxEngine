#pragma once

namespace vx
{
	class MeshFile;
}

class ArrayAllocator;

#include <vxEngineLib/Task.h>
#include <mutex>
#include <vxLib/StringID.h>
#include <vxLib/Container/sorted_array.h>
#include <vxEngineLib/Reference.h>
#include <vxLib/File/FileHeader.h>
#include <vxEngineLib/Pool.h>

class TaskLoadMesh : public Task
{
	std::mutex* m_mutexLoadedFiles;
	std::mutex* m_mutexCreateMesh;
	vx::StringID m_sid;
	vx::sorted_array<vx::StringID, Reference<vx::MeshFile>>* m_sortedMeshes;
	vx::Pool<ReferenceCounted<vx::MeshFile>>* m_poolMesh;
	ArrayAllocator* m_allocatorMeshData;
	vx::FileHeader m_fileHeader;
	const u8* m_fileData;
	u32 m_dataSize;

	TaskReturnType createMesh(ReferenceCounted<vx::MeshFile>** ptr);

	TaskReturnType runImpl() override;

public:
	explicit TaskLoadMesh(shared_ptr<Event> &&evt);
	~TaskLoadMesh();

	f32 getTimeMs() const override;
};