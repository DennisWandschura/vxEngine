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

	memcpy(m_buffer + m_size, text, size);

	m_size += size;
	m_buffer[m_size] = '\0';
}