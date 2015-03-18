#pragma once

namespace Component
{
	struct Actor;
}

#include "Condition.h"

class ConditionHasDestination : public Condition
{
	const Component::Actor* m_pActor;

public:
	explicit ConditionHasDestination(const Component::Actor* pActor);

	U8 test() const override;
};

class ConditionHasNoDestination : public Condition
{
	const Component::Actor* m_pActor;

public:
	explicit ConditionHasNoDestination(const Component::Actor* pActor);

	U8 test() const override;
};