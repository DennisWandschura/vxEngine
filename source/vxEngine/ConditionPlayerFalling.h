#pragma once

struct EntityHuman;

#include "Condition.h"

class ConditionPlayerFalling : public Condition
{
	EntityHuman* m_human;

public:
	explicit ConditionPlayerFalling(EntityHuman* human);
	~ConditionPlayerFalling();

	u8 test() const override;
};

class ConditionPlayerNotFalling : public Condition
{
	EntityHuman* m_human;

public:
	explicit ConditionPlayerNotFalling(EntityHuman* human);
	~ConditionPlayerNotFalling();

	u8 test() const override;
};