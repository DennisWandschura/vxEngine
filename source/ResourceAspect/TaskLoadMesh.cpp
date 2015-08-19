/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "TaskLoadMesh.h"
#include <vxResourceAspect/ResourceManager.h>
#include <vxLib/ScopeGuard.h>
#include <vxLib/File/File.h>
#include <vxEngineLib/MeshFile.h>
#include <vxLib/File/FileHeader.h>
#include <vxEngineLib/CpuTimer.h>
#include <fstream>

TaskLoadMesh::TaskLoadMesh(TaskLoadMeshDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.m_meshManager->getScratchAllocator(), desc.m_meshManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_filename(std::move(desc.m_filename)),
	m_meshManager(desc.m_meshManager),
	m_sid(desc.m_sid)
{
}

TaskLoadMesh::~TaskLoadMesh()
{

}

TaskReturnType TaskLoadMesh::runImpl()
{
	//CpuTimer timer;

	auto ptr = m_meshManager->find(m_sid);
	if (ptr != nullptr)
	{
		return TaskReturnType::Success;
	}

	managed_ptr<u8[]> fileData;
	u32 fileDataSize = 0;

	if (!loadFromFile(&fileData, &fileDataSize))
	{
		return TaskReturnType::Failure;
	}

	SCOPE_EXIT
	{
		std::unique_lock<std::mutex> scratchLock;
		auto scratchAlloc = m_meshManager->lockScratchAllocator(&scratchLock);
		fileData.clear();
	};

	const u8* dataBegin = nullptr;
	u32 dataSize = 0;
	u64 crc = 0;
	u32 version = 0;
	if (!readAndCheckHeader(fileData.get(), fileDataSize, &dataBegin, &dataSize, &crc, &version))
	{
		return TaskReturnType::Failure;
	}

	auto entry = m_meshManager->insertEntry(m_sid, std::move(m_filename), version);
	if (!entry)
	{
		VX_ASSERT(false);
		return TaskReturnType::Failure;
	}

	std::unique_lock<std::mutex> dataLock;
	auto dataAllocator = m_meshManager->lockDataAllocator(&dataLock);
	entry->loadFromMemory(dataBegin, dataSize, dataAllocator);

	return TaskReturnType::Success;
}

f32 TaskLoadMesh::getTimeMs() const
{
	return 0.0001f;
}