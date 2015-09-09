#pragma once

#include <vxLib/File/File.h>

class Logfile
{
	vx::File m_file;
	char* m_buffer;
	u32 m_capacity;
	u32 m_size;

	void flush();
	void copyToBuffer(const char* str, u32 siz);

public:
	Logfile();
	~Logfile();

	bool create(const char* filename);
	void close();

	void append(char c);
	void append(const char* text);
	void append(const char* text, u32 size);
};