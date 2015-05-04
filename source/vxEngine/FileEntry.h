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
#pragma once

#include <vxLib/types.h>

enum class FileType : U8;

#include <string>

class FileEntry
{
	static const U8 s_bufferSize = 31u;

	char m_file[s_bufferSize];
	FileType m_type;

public:
	FileEntry();
	FileEntry(const char *file, FileType t);

	template<size_t SIZE>
	FileEntry(const char(&file)[SIZE], FileType t)
	{
		static_assert(SIZE <= s_bufferSize, "Array too large !");
		for (U32 i = 0; i < SIZE; ++i)
		{
			m_file[i] = tolower(file[i]);
		}
	}

	FileEntry(const FileEntry &rhs);
	FileEntry(FileEntry &&rhs);

	FileEntry& operator=(const FileEntry &rhs);
	FileEntry& operator=(FileEntry &&rhs);

	FileType getType() const noexcept{ return m_type; }
	const char* getString() const noexcept { return m_file; }
};