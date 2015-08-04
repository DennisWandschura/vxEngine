#pragma once

#include <vxLib/types.h>

class CpuTimer
{
	static s64 s_frequency;

	s64 m_start;

public:
	CpuTimer();
	~CpuTimer();

	f32 getTimeMs() const;
};