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
#ifndef __VX_FILE_H
#define __VX_FILE_H
#pragma once

#include <vxLib\types.h>

enum class FileAccess : u32
{
	Read = 0x80000000L,
	Write = 0x40000000L,
	Read_Write = Read | Write
};

class File
{
	void *m_pFile;

public:
	File();
	~File();

	bool create(const char* file, FileAccess access);
	bool open(const char *file, FileAccess access);
	bool close();

	bool read(void *ptr, u32 size);

	template<typename T>
	bool read(T &value)
	{
		return read(&value, sizeof(T));
	}

	bool write(const void *ptr, u32 size, u32 *pWrittenBytes);

	template<typename T>
	bool write(const T &value)
	{
		return write(&value, sizeof(T), nullptr);
	}

	template<typename T>
	bool write(const T *ptr, u32 count)
	{
		return write(ptr, sizeof(T) * count, nullptr);
	}

	u32 getSize() const;
};
#endif