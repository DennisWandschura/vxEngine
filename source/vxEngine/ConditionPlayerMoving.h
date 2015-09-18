#pragma once

struct EntityHuman;

#include "Condition.h"

class ConditionPlayerMoving : public Condition
{
	EntityHuman* m_human;

public:
	explicit ConditionPlayerMoving(EntityHuman* human);
	~ConditionPlayerMoving();

	u8 test() const override;
};

class ConditionPlayerNotMoving : public Condition
{
	EntityHuman* m_human;

public:
	explicit ConditionPlayerNotMoving(EntityHuman* human);
	~ConditionPlayerNotMoving();

	u8 test() const override;
};