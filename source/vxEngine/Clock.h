#pragma once

#include <vxLib/types.h>

class Clock
{
	static U64 s_frequency;

	U64 m_startTime;

public:
	Clock();
	Clock(const Clock &rhs);
	Clock(Clock &&rhs);

	U64 getTime() const;

	static U64 getFrequency();
};