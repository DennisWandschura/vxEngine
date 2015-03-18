#pragma once

#include "Condition.h"

template<typename Comp>
class DistanceCondition
{
	F32 m_distance;

public:
	explicit DistanceCondition(F32 distance)
		:m_distance(distance){}

	U8 test(F32 distance)
	{
		return Comp()(m_distance, distance);
	}
};