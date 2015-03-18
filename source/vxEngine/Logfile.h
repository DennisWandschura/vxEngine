#pragma once

class Clock;

#include <fstream>
#include <vxLib/types.h>

class Logfile
{
	std::ofstream m_file;
	const Clock &m_clock;
	U64 m_lastTime;
	U32 m_record;
	char m_timeBuffer[16];
	U8 m_created;

	void convertTime(U64 time);
	void createStyleSheet(const char *stylesheet);

	void textout(const char *text);
	void fTextout(const char *text, ...);
	void fTextout(const char *text, char *args);

public:
	enum Type:U8{Normal, Warning, Error};

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