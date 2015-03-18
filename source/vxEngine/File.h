#ifndef __VX_FILE_H
#define __VX_FILE_H
#pragma once

#include <vxLib\types.h>

namespace FileAccess
{
	enum FileAccess : U8
	{
		access = 0
	};

	static const FileAccess Read = (FileAccess)0;
	static const FileAccess Write = (FileAccess)1;
	static const FileAccess Read_Write = (FileAccess)2;
}

class File
{
	void *m_pFile;

public:
	File();
	~File();

	bool open(const char *file, FileAccess::FileAccess access);
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