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

#include <vxLib/types.h>

#include <vxLib/File/FileHandle.h>

namespace vx
{
	enum class FileType : u8 { Invalid, Mesh, Texture, Material, Scene, Fbx };

	enum class FileStatus : u8
	{
		Exists, Loaded
	};

	class FileEntry
	{
		vx::FileHandle m_fileHandle;
		FileType m_type;

	public:
		FileEntry();
		FileEntry(const char *file, FileType t);

		template<size_t SIZE>
		FileEntry(const char(&file)[SIZE], FileType t)
			:m_fileHandle(file),
			m_type(t)
		{
		}

		FileEntry(const FileEntry &rhs);
		FileEntry(FileEntry &&rhs);

		FileEntry& operator=(const FileEntry &rhs);
		FileEntry& operator=(FileEntry &&rhs);

		FileType getType() const noexcept{ return m_type; }
		const char* getString() const noexcept{ return m_fileHandle.m_string; }
		const vx::StringID& getSid() const { return m_fileHandle.m_sid; }
	};
}