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
#include "Logfile.h"
#include "Clock.h"
#include <cstdarg>

Logfile::Logfile(Clock &clock)
	:m_file(),
	m_clock(clock),
	m_lastTime(0),
	m_record(0),
	m_timeBuffer(),
	m_created(0)
{
}

Logfile::~Logfile()
{
	if (m_created != 0)
		close();
}

bool Logfile::create(const char *filename)
{
	m_file.open(filename);
	if (!m_file.is_open())
		return false;

	m_created = 1;
	sprintf_s(m_timeBuffer, "00:00:00 ");

	auto stylesheet = "Stylesheet.xls";
	textout("<?xml-stylesheet type='text/xsl' href='");
	textout(stylesheet);
	textout("' ?><log>\n");

	createStyleSheet(stylesheet);

	return true;
}

void Logfile::close()
{
	textout("</log>");
	m_file.flush();
	m_file.close();
}

void Logfile::textout(const char *text)
{
	m_file << text;
}

void Logfile::fTextout(const char *text, ...)
{
	va_list argList;
	va_start(argList, text);

	fTextout(text, argList);

	va_end(argList);
}

void Logfile::fTextout(const char *text, char *args)
{
	const U32 buffersize = 511;
	static char sBuffer[buffersize + 1];

	vsnprintf_s(sBuffer, buffersize, text, args);
	sBuffer[buffersize] = '\0';
	textout(sBuffer);
}

void Logfile::writeEntry(const char *text, const char *file, const char *function, int line, Type type, bool bold)
{
	auto currentTime = m_clock.getTime();

	if ((currentTime - m_lastTime) > 1000000)
	{
		convertTime(currentTime);
		m_lastTime = currentTime;
	}

	fTextout("<entry><time>%s</time><record>%u</record>", m_timeBuffer, ++m_record);

	switch (type)
	{
	case(Type::Normal) :
	{
		if (bold)
			textout("<type>InformationBold</type>");
		else
			textout("<type>Information</type>");
	}break;
	case(Type::Warning) :
		textout("<type>Warning</type>");
		break;
	case(Type::Error) :
		textout("<type>Error</type>");
		break;
	default:
		textout("<type></type>");
		break;
	}

	textout("<description>");
	textout(text);
	textout("</description><file>");
	textout(file);
	textout("</file><function>");
	textout(function);
	fTextout("</function><line>%d</line></entry>\n", line);

	//m_file << "<entry><time>" << m_timeBuffer << "</time><record>0</record><type>Information</type><description>" << text << "</description><file>test.cpp</file><function>test()</function><line>1337</line></entry>\n";
}

void Logfile::fWriteEntry(const char *file, const char *function, int line, Type type, bool bold,const char *text , ...)
{
	auto currentTime = m_clock.getTime();

	if ((currentTime - m_lastTime) > 1000000)
	{
		convertTime(currentTime);
		m_lastTime = currentTime;
	}

	fTextout("<entry><time>%s</time><record>%u</record>", m_timeBuffer, ++m_record);

	switch (type)
	{
	case(Type::Normal) :
	{
		if (bold)
			textout("<type>InformationBold</type>");
		else
			textout("<type>Information</type>");
	}break;
	case(Type::Warning) :
		textout("<type>Warning</type>");
		break;
	case(Type::Error) :
		textout("<type>Error</type>");
		break;
	default:
		textout("<type></type>");
		break;
	}

	textout("<description>");

	va_list args;
	va_start(args, text);
	fTextout(text, args);
	va_end(args);

	textout("</description><file>");
	textout(file);
	textout("</file><function>");
	textout(function);
	fTextout("</function><line>%d</line></entry>\n", line);
}

void Logfile::convertTime(U64 time)
{
	const double inv = 1.0 / 60.0;
	const double inv2 = inv * inv;

	const auto frequency = Clock::getFrequency();

	auto microseconds = (time * 1000000) / frequency;

	auto elapsedSeconds = microseconds * 1.0e-6;
	const auto hour = elapsedSeconds * inv2;
	const auto minute = (hour - (U64)hour) * 60.0;
	const auto seconds = (minute - (U64)minute) * 60.0;

	sprintf_s(m_timeBuffer, "%02llu:%02llu:%02u ", static_cast<U64>(hour), static_cast<U64>(minute), static_cast<U32>(seconds));
}

void Logfile::createStyleSheet(const char *stylesheet)
{
	const char *text = "<?xml version=\"1.0\" encoding=\"iso-8859-1\"?><xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
		"<xsl:output method=\"html\"  encoding=\"utf-16\"/><xsl:template match=\"log\"><head><title>Log</title><style type=\"text/css\">"
		"body{ text-align: left; width: 99%%;  font-family: Verdana, sans-serif; }table{ border: none;  border-collapse: separate;  width: 100%%; }"
		"tr.title td{ font-size: 24px;  font-weight: bold; }th{ background: #d0d0d0;  font-weight: bold;  font-size: 10pt;  text-align: left; } tr{ background: #eeeeee}"
		"td, th{ font-size: 8pt;  padding: 1px;  border: none; }tr.info td{}tr.warning td{background-color:yellow;color:black}tr.error td{background-color:red;color:black}"
		"span {text-decoration:underline}a:hover{text-transform:uppercase;color: #9090F0;}</style></head>"
		"<body><table><tr class=\"title\">"
		"<td colspan=\"7\">Log</td></tr><tr><td colspan=\"2\">infos</td>"
		"<td colspan=\"5\"><xsl:value-of select=\"count(entry[type='Information'])\"/></td></tr>"
		"<tr><td colspan=\"2\">warnings</td>"
		"<td colspan=\"5\"><xsl:value-of select=\"count(entry[type='Warning'])\"/></td></tr>"
		"<tr><td colspan=\"2\">errors</td><td colspan=\"5\"><xsl:value-of select=\"count(entry[type='Error'])\"/></td></tr>"
		"<tr><th width=\"20\">#</th>"
		"<th>Type</th>"
		"<th>Time</th>"
		"<th>Description</th>"
		"<th>File</th>"
		"<th>Function</th>"
		"<th width=\"50\">Line</th></tr><xsl:apply-templates/></table></body></xsl:template>"
		"<xsl:template match=\"entry\"><xsl:choose>"
		"<xsl:when test=\"type='Information'\"><tr id=\"info\" class=\"info\"><td><xsl:value-of select=\"record\"/></td><td></td><xsl:call-template name=\"row\"/></tr></xsl:when>"
		"<xsl:when test=\"type='Warning'\"><tr id=\"warning\" class=\"warning\"><td><xsl:value-of select=\"record\"/></td><td>Warning</td><xsl:call-template name=\"row\"/></tr></xsl:when>"
		"<xsl:when test=\"type='Error'\"><tr id=\"error\" class=\"error\"><td><xsl:value-of select=\"record\"/></td><td>ERROR</td><xsl:call-template name=\"row\"/></tr></xsl:when>"
		"</xsl:choose></xsl:template>"
		"<xsl:template name=\"row\">"
		"<td id=\"time\"><xsl:value-of select=\"time\"/></td>"
		"<td id=\"description\"><xsl:value-of select=\"description\"/></td>"
		"<td id=\"file\"><xsl:value-of select=\"file\"/></td>"
		"<td><xsl:value-of select=\"function\"/></td>"
		"<td><xsl:value-of select=\"line\"/></td>"
		"</xsl:template></xsl:stylesheet>";

	FILE *pFile;
	fopen_s(&pFile, stylesheet, "w");
	fprintf(pFile, text);
	fflush(pFile);
	fclose(pFile);
}