#pragma once

class File;
class SceneFile;
class EditorScene;

namespace vx
{
	class StackAllocator;
}

#include <vxLib/types.h>

class FileFactory
{
public:
	static bool save(File* file, const EditorScene &data);

	static bool save(const char* file, const SceneFile &data);
	static bool save(File* file, const SceneFile &data);

	static bool load(const char* file, SceneFile* data, vx::StackAllocator* allocator);
	static bool load(File* file, SceneFile* data, vx::StackAllocator* allocator);
	static bool load(const U8* ptr, SceneFile* data);
};