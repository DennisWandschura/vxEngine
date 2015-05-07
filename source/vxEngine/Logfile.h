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

class Clock;

#include <fstream>
#include <vxLib/types.h>

class Logfile
{
	std::ofstream m_file;
	const Clock &m_clock;
	u64 m_lastTime;
	u32 m_record;
	char m_timeBuffer[16];
	u8 m_created;

	void convertTime(u64 time);
	void createStyleSheet(const char *stylesheet);

	void textout(const char *text);
	void fTextout(const char *text, ...);
	void fTextout(const char *text, char *args);

public:
	enum Type:u8{Normal, Warning, Error};

	explicit Logfile(Clock &clock);
	~Logfile();

	bool create(const char *filename);
	void close();

	void writeEntry(const char *text, const char *file, const char *function, int line, Type type = Type::Normal, bool bold = false);
	void fWriteEntry(const char *file, const char *function, int line, Type type, bool bold, const char *text, ...);
};

#define LOG(LOGFILE, TEXT, BOLD) \
LOGFILE.writeEntry(TEXT, __FILE__, __FUNCTION__, __LINE__, Logfile::Normal, BOLD)

#define LOG_WARNING(LOGFILE, TEXT, BOLD) \
LOGFILE.writeEntry(TEXT, __FILE__, __FUNCTION__, __LINE__, Logfile::Warning, BOLD)

#define LOG_ERROR(LOGFILE, TEXT, BOLD) \
LOGFILE.writeEntry(TEXT, __FILE__, __FUNCTION__, __LINE__, Logfile::Error, BOLD)

#define LOG_ARGS(LOGFILE, TEXT, BOLD, ...) \
LOGFILE.fWriteEntry(__FILE__, __FUNCTION__, __LINE__, Logfile::Normal, BOLD, TEXT, __VA_ARGS__)

#define LOG_WARNING_ARGS(LOGFILE, TEXT, BOLD, ...) \
LOGFILE.fWriteEntry(__FILE__, __FUNCTION__, __LINE__, Logfile::Warning, BOLD, TEXT, __VA_ARGS__)

#define LOG_ERROR_ARGS(LOGFILE, TEXT, BOLD, ...) \
LOGFILE.fWriteEntry(__FILE__, __FUNCTION__, __LINE__, Logfile::Error, BOLD, TEXT, __VA_ARGS__)