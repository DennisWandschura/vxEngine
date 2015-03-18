#pragma once

namespace Component
{
	struct Actor;
}

#include "Condition.h"

class ConditionHasPath : public Condition
{
	const Component::Actor* m_pActor;

public:
	explicit ConditionHasPath(const Component::Actor* pActor);

	U8 test() const override;
};