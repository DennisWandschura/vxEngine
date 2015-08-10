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
#include "TaskLoadMaterial.h"
#include <vxResourceAspect/ResourceManager.h>
#include <vxEngineLib/Material.h>
#include <vxResourceAspect/MaterialFactory.h>
#include <vxResourceAspect/FileEntry.h>
#include "TaskLoadTexture.h"
#include <vxEngineLib/TaskManager.h>

TaskLoadMaterial::TaskLoadMaterial(TaskLoadMaterialDesc &&desc)
	:Task(std::move(desc.evt)),
	m_fileNameWithPath(std::move(desc.m_fileNameWithPath)),
	m_materialManager(desc.m_materialManager),
	m_textureManager(desc.m_textureManager),
	m_sid(desc.m_sid),
	m_taskManager(desc.m_taskManager)
{

}

TaskLoadMaterial::~TaskLoadMaterial()
{

}

TaskReturnType TaskLoadMaterial::runImpl()
{
	auto ptr = m_materialManager->find(m_sid);
	if (ptr.get() != nullptr)
	{
		return TaskReturnType::Success;
	}

	std::vector<vx::FileEntry> missingFiles;

	Material material;

	MaterialFactoryLoadDescNew loadMaterialDesc;
	loadMaterialDesc.fileNameWithPath = m_fileNameWithPath.c_str();
	loadMaterialDesc.material = &material;
	loadMaterialDesc.missingFiles = &missingFiles;
	loadMaterialDesc.m_textureManager = m_textureManager;

	if (!MaterialFactory::load(loadMaterialDesc))
	{
		if (missingFiles.size() != 0)
		{
			std::vector<shared_ptr<Event>> events;
			for(auto &it : missingFiles)
			{
				auto evt = shared_ptr<Event>(new Event());
				TaskLoadTextureDesc loadTexDesc;
				{
					it.getString(),
					evt,
					it.getSid(),
					m_textureManager;
				};

				auto task = new TaskLoadTexture(std::move(loadTexDesc));

				m_taskManager->pushTask(task, false);

				events.push_back(evt);
			}

			setEventList(&events);
			setTimeoutTime(500.0f);

			return TaskReturnType::Retry;
		}
		else
		{
			VX_ASSERT(false);
			return TaskReturnType::Failure;
		}
	}

	auto ref = m_materialManager->insertEntry(m_sid, std::move(material));
	if (ref.get() == nullptr)
	{
		return TaskReturnType::Failure;
	}

	return TaskReturnType::Success;
}