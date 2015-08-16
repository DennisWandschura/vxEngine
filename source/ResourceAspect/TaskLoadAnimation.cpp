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
#include "TaskLoadAnimation.h"
#include <vxEngineLib/AnimationFile.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxLib/ScopeGuard.h>

TaskLoadAnimation::TaskLoadAnimation(TaskLoadAnimationDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.m_animationManager->getScratchAllocator(), desc.m_animationManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_animationManager(desc.m_animationManager),
	m_sid(desc.m_sid)
{

}

TaskLoadAnimation::~TaskLoadAnimation()
{

}

TaskReturnType TaskLoadAnimation::runImpl()
{
	auto ptr = m_animationManager->find(m_sid);
	if (ptr != nullptr)
	{
		return TaskReturnType::Success;
	}

	managed_ptr<u8[]> fileData;
	u32 fileSize = 0;
	if (!loadFromFile(&fileData, &fileSize))
	{
		return TaskReturnType::Failure;
	}

	SCOPE_EXIT
	{
		std::unique_lock<std::mutex> scratchLock;
		auto scratchAlloc = m_animationManager->lockScratchAllocator(&scratchLock);
		fileData.clear();
	};

	const u8* dataBegin = nullptr;
	u32 dataSize = 0;
	u64 headerCrc = 0;
	if (!readAndCheckHeader(fileData.get(), fileSize, &dataBegin, &dataSize, &headerCrc))
	{
		return TaskReturnType::Failure;
	}

	vx::AnimationFile animFile(vx::AnimationFile::getGlobalVersion());

	animFile.loadFromMemory(dataBegin, fileSize, nullptr);
	auto crc = animFile.getCrc();

	if (headerCrc != crc)
	{
		//	printf("wrong crc: %llu %llu\n", headerTop.crc, crc);
		return TaskReturnType::Failure;
	}

	auto ref = m_animationManager->insertEntry(m_sid, std::move(animFile.getAnimation()));
	if (ref == nullptr)
	{
		return TaskReturnType::Failure;
	}

	return TaskReturnType::Success;
}

f32 TaskLoadAnimation::getTimeMs() const
{
	return 0.0f;
}