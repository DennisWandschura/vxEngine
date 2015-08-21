#pragma once

#include <vxLib/File/File.h>

class Logfile
{
	vx::File m_file;
	char* m_buffer;
	u32 m_capacity;
	u32 m_size;

	void flush();

public:
	Logfile();
	~Logfile();

	bool create(const char* filename);
	void close();

	void append(const char* text);
	void append(const char* text, u32 size);
};