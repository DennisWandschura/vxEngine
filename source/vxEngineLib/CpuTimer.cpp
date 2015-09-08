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

#include <vxEngineLib/CpuTimer.h>
#include <Windows.h>

s64 CpuTimer::s_frequency{0};

CpuTimer::CpuTimer()
	:m_start(0)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&m_start);

	if (s_frequency == 0)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&s_frequency);
	}
}

CpuTimer::~CpuTimer()
{

}

void CpuTimer::reset()
{
	LARGE_INTEGER start;
	QueryPerformanceCounter(&start);

	m_start = start.QuadPart;
}

f32 CpuTimer::getTimeMs() const
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	auto time = (end.QuadPart - m_start) * 1000000 / s_frequency;

	return f32(time * 1.0e-6);
}
