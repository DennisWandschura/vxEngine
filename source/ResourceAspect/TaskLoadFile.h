#pragma once

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

namespace vx
{
	class StackAllocator;
}

#include <vxEngineLib/Task.h>
#include <string>

class TaskLoadFile : public Task
{
	vx::StackAllocator* m_scratchAllocator;
	std::mutex* m_mutex;

protected:
	std::string m_fileNameWithPath;

	bool loadFromFile(u8** ptr, u32* fileSize);
	bool readAndCheckHeader(const u8* fileData, u32 fileSize, const u8** dataBegin, u32* dataSize, u64* crc);

	TaskLoadFile(std::string &&fileNameWithPath, vx::StackAllocator* scratchAllocator, std::mutex* mutex, shared_ptr<Event> &&evt);

public:
	virtual ~TaskLoadFile();
};