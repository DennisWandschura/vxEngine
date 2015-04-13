#ifndef __VX_FILE_H
#define __VX_FILE_H
#pragma once

#include <vxLib\types.h>

enum class FileAccess : U32
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

	bool read(void *ptr, U32 size);

	template<typename T>
	bool read(T &value)
	{
		return read(&value, sizeof(T));
	}

	bool write(const void *ptr, U32 size, U32 *pWrittenBytes);

	template<typename T>
	bool write(const T &value)
	{
		return write(&value, sizeof(T), nullptr);
	}

	template<typename T>
	bool write(const T *ptr, U32 count)
	{
		return write(ptr, sizeof(T) * count, nullptr);
	}

	// returns size of file, on failure it returns zero
	U32 getSize() const;
};
#endif