#include "ConditionPlayerFalling.h"
#include "Entity.h"

ConditionPlayerFalling::ConditionPlayerFalling(EntityHuman* human)
	:m_human(human)
{

}

ConditionPlayerFalling::~ConditionPlayerFalling()
{

}

u8 ConditionPlayerFalling::test() const
{
	auto cmp = m_human->m_state & (1 << (u32)EntityHuman::State::Falling);

	return (cmp != 0);
}

ConditionPlayerNotFalling::ConditionPlayerNotFalling(EntityHuman* human)
	:m_human(human)
{

}

ConditionPlayerNotFalling::~ConditionPlayerNotFalling()
{

}

u8 ConditionPlayerNotFalling::test() const
{
	auto cmp = m_human->m_state & (1 << (u32)EntityHuman::State::Falling);

	return (cmp == 0);
}