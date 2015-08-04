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

f32 CpuTimer::getTimeMs() const
{
	LARGE_INTEGER end;
	QueryPerformanceCounter(&end);

	auto time = (end.QuadPart - m_start) * 1000000 / s_frequency;

	return f32(time * 1.0e-6);
}
