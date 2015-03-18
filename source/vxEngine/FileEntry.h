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