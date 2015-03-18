#pragma once

namespace Component
{
	struct Physics;
}

#include "Action.h"
#include "Arrive.h"

class ActionGoToPosition : public Action
{
	Component::Input* m_pInput{nullptr};
	Arrive m_arrive;
	U16 m_arrived{0};

public:
	ActionGoToPosition(Component::Input* pInput, Component::Physics* pPhysics);

	void setTarget(const vx::float3 &targetPos);

	void run() override;
	bool isComplete() const override;
};