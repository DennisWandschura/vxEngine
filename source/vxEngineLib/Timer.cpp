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
#include <vxEngineLib/Timer.h>
#include <Windows.h>

u64 Timer::s_frequency{ 0 };

Timer::Timer()
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

Timer::Timer(const Timer &rhs)
	:m_startTime(rhs.m_startTime)
{
}

Timer::Timer(Timer &&rhs)
	: m_startTime(rhs.m_startTime)
{
}

void Timer::reset()
{
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);

	m_startTime = start.QuadPart;
}

u64 Timer::getTime() const
{
	LARGE_INTEGER current;
	QueryPerformanceCounter(&current);

	return current.QuadPart - m_startTime;
}

f32 Timer::getTimeInMs() const
{
	auto timeInMicroseconds = getTime();

	timeInMicroseconds *= 1000000;
	timeInMicroseconds /= s_frequency;

	return (f32)((f64)timeInMicroseconds * 0.001f);
}

u64 Timer::getFrequency()
{
	return s_frequency;
}