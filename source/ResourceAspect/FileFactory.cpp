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
#include <vxResourceAspect/FileFactory.h>
#include <vxResourceAspect/SceneFile.h>
#include <vxResourceAspect/SceneFactory.h>
#include <vxLib/File/File.h>
#include <vxLib/File/FileHeader.h>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/ScopeGuard.h>
#include <vxEngineLib/FileFactory.h>

bool FileFactory::load(const char* file, SceneFile* data, vx::StackAllocator* scratchAllocator, vx::Allocator* allocator)
{
	vx::File f;
	if (!f.open(file, vx::FileAccess::Read))
		return false;

	return load(&f, data, scratchAllocator, allocator);
}

bool FileFactory::load(vx::File* file, SceneFile* data, vx::StackAllocator* scratchAllocator, vx::Allocator* allocator)
{
	auto fileSize = file->getSize();

	auto marker = scratchAllocator->getMarker();

	SCOPE_EXIT
	{
		scratchAllocator->clear(marker);
	};

	u8* ptr = allocator->allocate(fileSize, 8);
	if (!file->read(ptr, fileSize))
		return false;

	file->close();
	
	return load(ptr, fileSize, data, allocator);
}

bool FileFactory::load(const u8* ptr, u32 fileSize, SceneFile* data, vx::Allocator* allocator)
{
	vx::FileHeader headerTop = *(vx::FileHeader*)ptr;

	if (headerTop.magic != vx::FileHeader::s_magic)
		return false;

	auto dataPtr = ptr + sizeof(vx::FileHeader);
	ptr = data->loadFromMemory(dataPtr, fileSize, headerTop.version, allocator);

	auto currentCrc = data->getCrc();
	if (headerTop.crc != currentCrc)
		return false;

	vx::FileHeader headerBottom = *(vx::FileHeader*)ptr;
	if (headerBottom.magic != vx::FileHeader::s_magic)
		return false;

	return headerBottom.crc == currentCrc;
}