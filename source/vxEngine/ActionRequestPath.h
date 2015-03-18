#pragma once

namespace Component
{
	struct Actor;
}

#include "Action.h"

class ActionRequestPath : public Action
{
	Component::Actor* m_pActor{ nullptr };
public:
	explicit ActionRequestPath(Component::Actor* p);
	void run() override;
};