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

#include "TaskLoadFont.h"
#include <vxResourceAspect/ResourceManager.h>
#include <vxEngineLib/ParserNode.h>
#include <vxEngineLib/Graphics/Font.h>
#include <vxResourceAspect/ResourceAspect.h>

TaskLoadFont::TaskLoadFont(TaskLoadFontDesc &&desc)
	:TaskLoadFile(std::move(desc.m_fileNameWithPath), desc.m_fontManager->getScratchAllocator(), desc.m_fontManager->getScratchAllocatorMutex(), std::move(desc.evt)),
	m_filename(std::move(desc.m_filename)),
	m_fontManager(desc.m_fontManager),
	m_textureManager(desc.m_textureManager),
	m_sid(desc.m_sid),
	m_path(std::move(desc.m_path)),
	m_textureName(),
	m_waitingForTexture(false),
	m_resourceAspect(desc.m_resourceAspect)
{

}

TaskLoadFont::~TaskLoadFont()
{

}

bool TaskLoadFont::loadFontData()
{
	managed_ptr<u8[]> fileData;
	u32 fileSize = 0;
	if (!loadFromFile(&fileData, &fileSize))
	{
		return false;
	}

	// swap magic data with zero for string
	u8 tmpMagic = fileData[fileSize];
	fileData[fileSize] = '\0';

	Parser::Node root;
	root.create((char*)fileData.get());
	//root.createFromFile(m_fileNameWithPath.c_str());

	auto nodeMeta = root.get("meta");
	auto nodeTexture = root.get("texture");

	std::string metaFile;
	nodeMeta->as(&metaFile);

	nodeTexture->as(&m_textureName);

	fileData[fileSize] = tmpMagic;
	std::unique_lock<std::mutex> lck;
	m_fontManager->lockScratchAllocator(&lck);
	fileData.clear();
	lck.unlock();

	auto metaSz = metaFile.size();
	if (metaFile[metaSz - 1] == '\r')
	{
		metaFile.pop_back();
	}

	std::string atlasPath = m_path + "meta/" + metaFile;
	m_fileNameWithPath = atlasPath;
	if (!loadFromFile(&fileData, &fileSize))
	{
		return false;
	}

	if (!m_fontAtlas.loadFromMemory((char*)fileData.get()))
	{
		return false;
	}

	m_waitingForTexture = true;

	requestLoadTexture();

	return true;
}

void TaskLoadFont::requestLoadTexture()
{
	auto evt = Event::createEvent();
	vx::Variant arg;
	arg.u8 = 0;

	std::vector<Event> fileEvents;
	fileEvents.push_back(evt);
	setEventList(&fileEvents);

	m_resourceAspect->requestLoadFile(vx::FileEntry(m_textureName.c_str(), vx::FileType::Texture), arg, evt);
}

TaskReturnType TaskLoadFont::runImpl()
{
	if (m_waitingForTexture)
	{
		auto texSid = vx::make_sid(m_textureName.c_str());
		auto ptr = m_textureManager->find(texSid);

		if (ptr == nullptr)
		{
			VX_ASSERT(false);
		}

		return TaskReturnType::Success;
	}
	else
	{
		loadFontData();
		return TaskReturnType::WaitingForEvents;
	}
}

f32 TaskLoadFont::getTimeMs() const
{
	return 0.0f;
}