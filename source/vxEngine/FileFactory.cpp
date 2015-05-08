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
#include "FileFactory.h"
#include <vxLib/File/File.h>
#include "SceneFile.h"
#include <vxLib/File/FileHeader.h>
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/ScopeGuard.h>
#include "SceneFactory.h"

bool FileFactory::save(vx::File* file, const EditorScene &data)
{
	SceneFile sceneFile;
	SceneFactory::convert(data, &sceneFile);

	return save(file, sceneFile);
}

bool FileFactory::save(const char* file, const SceneFile &data)
{
	vx::File f;
	if (!f.create(file, vx::FileAccess::Write))
		return false;

	return save(&f, data);
}

bool FileFactory::save(vx::File* file, const SceneFile &data)
{
	vx::FileHeader header;
	header.magic = vx::FileHeader::s_magic;
	header.version = data.getVersion();
	header.crc = data.getCrc();

	file->write(header);
	return data.saveToFile(file);
}

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
	
	return load(ptr, data, allocator);
}

bool FileFactory::load(const u8* ptr, SceneFile* data, vx::Allocator* allocator)
{
	vx::FileHeader header = *(vx::FileHeader*)ptr;

	if (header.magic != vx::FileHeader::s_magic)
		return false;

	auto dataPtr = ptr + sizeof(vx::FileHeader);
	data->loadFromMemory(dataPtr, header.version, allocator);

	auto currentCrc = data->getCrc();

	return (header.crc == currentCrc);
}