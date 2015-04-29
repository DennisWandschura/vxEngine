#include "FileFactory.h"
#include "File.h"
#include "SceneFile.h"
#include "FileHeader.h"
#include <vxLib/Allocator/StackAllocator.h>
#include <vxLib/ScopeGuard.h>
#include "SceneFactory.h"

bool FileFactory::save(File* file, const EditorScene &data)
{
	SceneFile sceneFile;
	SceneFactory::convert(data, &sceneFile);

	return save(file, sceneFile);
}

bool FileFactory::save(const char* file, const SceneFile &data)
{
	File f;
	if (!f.create(file, FileAccess::Write))
		return false;

	return save(&f, data);
}

bool FileFactory::save(File* file, const SceneFile &data)
{
	FileHeader header;
	header.magic = FileHeader::s_magic;
	header.version = data.getVersion();
	header.crc = data.getCrc();

	file->write(header);
	return data.saveToFile(file);
}

bool FileFactory::load(const char* file, SceneFile* data, vx::StackAllocator* allocator)
{
	File f;
	if (!f.open(file, FileAccess::Read))
		return false;

	return load(&f, data, allocator);
}

bool FileFactory::load(File* file, SceneFile* data, vx::StackAllocator* allocator)
{
	auto fileSize = file->getSize();

	auto marker = allocator->getMarker();

	SCOPE_EXIT
	{
		allocator->clear(marker);
	};

	U8* ptr = allocator->allocate(fileSize, 8);
	if (!file->read(ptr, fileSize))
		return false;

	file->close();
	
	return load(ptr, data);
}

bool FileFactory::load(const U8* ptr, SceneFile* data)
{
	FileHeader header = *(FileHeader*)ptr;

	if (header.magic != FileHeader::s_magic)
		return false;

	auto dataPtr = ptr + sizeof(FileHeader);
	data->loadFromMemory(dataPtr, header.version);

	auto currentCrc = data->getCrc();

	return (header.crc == currentCrc);
}