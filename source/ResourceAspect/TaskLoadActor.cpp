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

#include "TaskLoadActor.h"
#include <vxEngineLib/ActorFile.h>
#include <vxResourceAspect/ResourceManager.h>
#include <vxResourceAspect/ResourceAspect.h>
#include <vxEngineLib/Actor.h>

TaskLoadActor::TaskLoadActor(TaskLoadActorDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.m_actorResManager->getScratchAllocator(), desc.m_actorResManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_fileName(std::move(desc.m_fileName)),
	m_sid(desc.m_sid),
	m_actorResManager(desc.m_actorResManager),
	m_resourceAspect(desc.m_resourceAspect)
{

}

TaskLoadActor::~TaskLoadActor()
{

}

TaskReturnType TaskLoadActor::runImpl()
{
	u32 fileSize = 0;
	managed_ptr<u8[]> fileData;
	if(!loadFromFile(&fileData, &fileSize))
		return TaskReturnType::Failure;

	const u8* dataBegin = nullptr;
	u32 dataSize = 0;
	u64 crc = 0;
	u32 version = 0;
	if (!readAndCheckHeader(fileData.get(), fileSize, &dataBegin, &dataSize, &crc, &version))
	{
		return TaskReturnType::Failure;
	}

	ActorFile actorFile(version);
	actorFile.loadFromMemory(dataBegin, dataSize, nullptr);

	vx::FileHandle handleMesh(actorFile.getMesh());
	vx::FileHandle handleMaterial(actorFile.getMaterial());

	auto result = TaskReturnType::Success;
	auto material = m_resourceAspect->getMaterial(handleMaterial.m_sid);

	std::vector<Event> events;
	if (material == nullptr)
	{
		auto evt = Event::createEvent();

		vx::Variant arg;
		arg.u64 = 0;
		m_resourceAspect->requestLoadFile(vx::FileEntry(handleMaterial.m_string, vx::FileType::Material), arg, evt);

		events.push_back(evt);

		result = TaskReturnType::Retry;
	}

	auto mesh = m_resourceAspect->getMesh(handleMesh.m_sid);
	if (mesh == nullptr)
	{
		auto evt = Event::createEvent();

		vx::Variant arg;
		arg.u64 = 0;
		m_resourceAspect->requestLoadFile(vx::FileEntry(handleMesh.m_string, vx::FileType::Mesh), arg, evt);

		events.push_back(evt);

		result = TaskReturnType::Retry;
	}

	if (!events.empty())
	{
		setEventList(&events);
	}
	else
	{
		Actor actor;
		actor.m_material = handleMaterial.m_sid;
		actor.m_mesh = handleMesh.m_sid;

		m_actorResManager->insertEntry(m_sid, std::move(m_fileName), std::move(actor));
	}

	return result;
}