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

#include <vxEngineLib/Logfile.h>
#include <cstring>

Logfile::Logfile()
	:m_file(),
	m_buffer(nullptr),
	m_capacity(0),
	m_size(0)
{

}

Logfile::~Logfile()
{
	close();
}

bool Logfile::create(const char* filename)
{
	bool result = true;

	if (!m_file.isOpen())
	{
		result = m_file.create(filename, vx::FileAccess::Write);

		m_buffer = new char[2048];
		m_capacity = 2048;
		m_size = 0;
	}

	return result;
}

void Logfile::close()
{
	if (m_file.isOpen())
	{
		if (m_size != 0)
		{
			flush();
		}
		m_file.close();
	}

	if (m_buffer)
	{
		delete[]m_buffer;
		m_buffer = nullptr;
		m_capacity = 0;
	}
}

void Logfile::flush()
{
	m_file.write(m_buffer, m_size);
	memset(m_buffer, '\0', m_size);
	m_size = 0;
}

void Logfile::copyToBuffer(const char* str, u32 size)
{
	memcpy(m_buffer + m_size, str, size);
	m_size += size;
	m_buffer[m_size] = '\0';
}

void Logfile::append(char c)
{
	if ((m_size + 1) >= m_capacity)
	{
		flush();
	}

	copyToBuffer(&c, 1);
}

void Logfile::append(const char* text)
{
	auto sz = strlen(text);
	append(text, sz);
}

void Logfile::append(const char* text, u32 size)
{
	if (size == 0)
		return;

	if((m_size + size) >= m_capacity)
	{
		flush();
	}

	copyToBuffer(text, size);
}