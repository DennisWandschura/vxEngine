#pragma once

namespace Component
{
	struct Physics;
	struct Actor;
}

class ActorData;

#include "Action.h"
#include "Arrive.h"
#include <vector>

class ActionFollowPath : public Action
{
	Component::Input* m_pInput{ nullptr };
	Arrive m_arrive;
	ActorData* m_pData{ nullptr };
	Component::Actor* m_pActor{ nullptr };
	U8 m_update{1};

public:
	ActionFollowPath(Component::Input* pInput, Component::Physics* pPhysics, Component::Actor* pActor);

	void run() override;
	bool isComplete() const override;

	void updateTargetPosition();
};