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
#include "TaskLoadFile.h"
#include <vxLib/File/File.h>
#include <vxEngineLib/ArrayAllocator.h>
#include <vxLib/File/FileHeader.h>

TaskLoadFile::TaskLoadFile(std::string &&fileNameWithPath, ArrayAllocator* scratchAllocator, std::mutex* mutex, Event &&evt)
	:Task(std::move(evt)),
	m_fileNameWithPath(std::move(fileNameWithPath)),
	m_scratchAllocator(scratchAllocator),
	m_mutex(mutex)
{

}

TaskLoadFile::~TaskLoadFile()
{

}

bool TaskLoadFile::loadFromFile(managed_ptr<u8[]>* outPtr, u32* outFileSize)
{
	vx::File f;
	if (!f.open(m_fileNameWithPath.c_str(), vx::FileAccess::Read))
	{
		//LOG_ERROR_ARGS(m_logfile, "Error opening file '%s'\n", false, file);
		printf("ERROR TaskLoadFile::loadFromFile opening file %s, %p\n", m_fileNameWithPath.c_str(), this);
		return false;
	}

	auto fileSize = f.getSize();
	VX_ASSERT(fileSize == static_cast<u32>(fileSize));
	VX_ASSERT(fileSize != 0);

	std::unique_lock<std::mutex> lock(*m_mutex);
	*outPtr = m_scratchAllocator->allocate<u8[]>(static_cast<u32>(fileSize), 4);
	if (outPtr->get() == nullptr)
	{
		return false;
	}
	lock.unlock();

	memset(outPtr->get(), 0, fileSize);

	if (!f.read(outPtr->get(), static_cast<u32>(fileSize)))
	{
		return false;
	}

	*outFileSize = static_cast<u32>(fileSize);

	return true;
}

bool TaskLoadFile::readAndCheckHeader(const u8* fileData, u32 fileSize, const u8** outDataBegin, u32* dataSize, u64* crc, u32* version)
{
	vx::FileHeader headerTop;
	memcpy(&headerTop, fileData, sizeof(vx::FileHeader));

	if (headerTop.s_magic != headerTop.magic)
		return false;

	auto dataBegin = fileData + sizeof(vx::FileHeader);
	auto fileDataEnd = fileData + fileSize;

	vx::FileHeader headerBottom;
	memcpy(&headerBottom, fileDataEnd - sizeof(vx::FileHeader), sizeof(vx::FileHeader));

	if (!headerBottom.isEqual(headerTop))
	{
		return false;
	}

	*outDataBegin = dataBegin;
	*dataSize = fileSize - sizeof(vx::FileHeader) * 2;
	*version = headerTop.version;
	*crc = headerTop.crc;

	return true;
}