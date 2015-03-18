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
	for (U32 i = 0; i < size; ++i)
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