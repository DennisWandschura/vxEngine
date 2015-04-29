#pragma once

struct EntityActor;

#include "Action.h"
#include "Arrive.h"

class ActionGoToPosition : public Action
{
	Arrive m_arrive;
	U16 m_arrived{0};

public:
	ActionGoToPosition(Component::Input* pInput, EntityActor* entity);

	void setTarget(const vx::float3 &targetPos);

	void run() override;
	bool isComplete() const override;
};