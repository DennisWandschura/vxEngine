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
#include "FileEntry.h"

FileEntry::FileEntry()
	:m_file(),
	m_type()
{
}

FileEntry::FileEntry(const char *file, FileType t)
	:m_file(),
	m_type(t)
{
	auto size = strlen(file);
	for (u32 i = 0; i < size; ++i)
	{
		m_file[i] = tolower(file[i]);
	}
}

FileEntry::FileEntry(const FileEntry &rhs)
	:m_file(),
	m_type(rhs.m_type)
{
	strcpy_s(m_file, rhs.m_file);
}

FileEntry::FileEntry(FileEntry &&rhs)
	:m_file(),
	m_type(rhs.m_type)
{
	strcpy_s(m_file, rhs.m_file);
}

FileEntry& FileEntry::operator=(const FileEntry &rhs)
{
	if (this != &rhs)
	{
		strcpy_s(m_file, rhs.m_file);
		m_type = rhs.m_type;
	}
	return *this;
}

FileEntry& FileEntry::operator=(FileEntry &&rhs)
{
	if (this != &rhs)
	{
		strcpy_s(m_file, rhs.m_file);
		m_type = rhs.m_type;
	}
	return *this;
}