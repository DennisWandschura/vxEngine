#include "ConditionPlayerMoving.h"
#include "Entity.h"

ConditionPlayerMoving::ConditionPlayerMoving(EntityHuman* human)
	:m_human(human)
{

}

ConditionPlayerMoving::~ConditionPlayerMoving()
{

}

u8 ConditionPlayerMoving::test() const
{
	auto cmp = m_human->m_state & (1 << (u32)EntityHuman::State::Walking);

	return (cmp != 0);
}

ConditionPlayerNotMoving::ConditionPlayerNotMoving(EntityHuman* human)
	:m_human(human)
{

}

ConditionPlayerNotMoving::~ConditionPlayerNotMoving()
{

}

u8 ConditionPlayerNotMoving::test() const
{
	auto cmp = m_human->m_state & (1 << (u32)EntityHuman::State::Walking);

	return (cmp == 0);
}