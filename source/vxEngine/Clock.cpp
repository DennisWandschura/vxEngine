#include "Clock.h"
#include <Windows.h>
#include <future>

U64 Clock::s_frequency{0};

Clock::Clock()
{
	if (s_frequency == 0)
	{
		LARGE_INTEGER f;
		QueryPerformanceFrequency(&f);

		s_frequency = f.QuadPart;
	}

	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);

	m_startTime = start.QuadPart;
}

Clock::Clock(const Clock &rhs)
	:m_startTime(rhs.m_startTime)
{
}

Clock::Clock(Clock &&rhs)
	: m_startTime(rhs.m_startTime)
{
}

U64 Clock::getTime() const
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);

	return current.QuadPart - m_startTime;
}

U64 Clock::getFrequency()
{
	return s_frequency;
}