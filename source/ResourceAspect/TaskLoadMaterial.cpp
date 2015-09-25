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
#include <vxEngineLib/FileEntry.h>
#include "TaskLoadTexture.h"
#include <vxEngineLib/TaskManager.h>

TaskLoadMaterial::TaskLoadMaterial(TaskLoadMaterialDesc &&desc)
	:Task(std::move(desc.evt)),
	m_fileNameWithPath(std::move(desc.m_fileNameWithPath)),
	m_filename(std::move(desc.m_filename)),
	m_materialManager(desc.m_materialManager),
	m_textureManager(desc.m_textureManager),
	m_sid(desc.m_sid),
	m_taskManager(desc.m_taskManager),
	m_textureFolder(desc.m_textureFolder),
	m_flipImage(desc.m_flipImage)
{

}

TaskLoadMaterial::~TaskLoadMaterial()
{

}

TaskReturnType TaskLoadMaterial::runImpl()
{
	auto ptr = m_materialManager->find(m_sid);
	if (ptr != nullptr)
	{
		return TaskReturnType::Success;
	}

	u32 missingFilesCount = 0;
	MissingTextureFile missingFiles[3];

	Material material(m_sid);

	MaterialFactoryLoadDescNew loadMaterialDesc;
	loadMaterialDesc.fileNameWithPath = m_fileNameWithPath.c_str();
	loadMaterialDesc.material = &material;
	loadMaterialDesc.missingFiles = missingFiles;
	loadMaterialDesc.missingFilesCount = &missingFilesCount;
	loadMaterialDesc.m_textureManager = m_textureManager;

	if (!MaterialFactory::load(loadMaterialDesc))
	{
		if (missingFilesCount != 0)
		{
			char fileNameWithPath[64];
			std::vector<Event> events;

			for (u32 i = 0; i < missingFilesCount; ++i)
			{
				auto &it = missingFiles[i];

				auto fileName = it.fileEntry.getString();

				auto returnCode = sprintf_s(fileNameWithPath, "%s%s", m_textureFolder, fileName);
				if (returnCode == -1)
				{
					VX_ASSERT(false);
				}

				auto evt = Event::createEvent();
				TaskLoadTextureDesc loadTexDesc
				{
					std::string(fileNameWithPath),
					std::string(fileName),
					evt,
					it.fileEntry.getSid(),
					m_textureManager,
					m_flipImage,
					it.srgb
				};

				auto task = new TaskLoadTexture(std::move(loadTexDesc));

				m_taskManager->pushTask(task);

				events.push_back(evt);
			}

			setEventList(&events);
			//setTimeoutTime(500.0f);

			return TaskReturnType::Retry;
		}
		else
		{
			printf("TaskLoadMaterial %llu %p Failure\n", m_sid.value, this);
			VX_ASSERT(false);
			return TaskReturnType::Failure;
		}
	}

	auto ref = m_materialManager->insertEntry(m_sid, std::move(m_filename),std::move(material));
	if (ref == nullptr)
	{
		return TaskReturnType::Failure;
	}

	printf("TaskLoadMaterial: %s %llu\n", m_filename.c_str(), m_sid.value);
	return TaskReturnType::Success;
}