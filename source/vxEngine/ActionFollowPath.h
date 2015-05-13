#pragma once

#include "Action.h"
#include <vector>
#include <vxLib/math/Vector.h>
#include "Arrive.h"

class ActionFollowPath : public Action
{
	EntityActor* m_entity;
	Arrive m_arrive;
	std::vector<vx::float3> m_path;
	bool m_arrived;

public:
	ActionFollowPath(EntityActor* entity);

	void run() override;

	bool isComplete() const override;
};