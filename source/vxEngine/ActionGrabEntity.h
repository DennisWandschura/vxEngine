#pragma once

struct EntityHuman;
struct EntityDynamic;

namespace physx
{
	class PxJoint;
}

#include "ActionUseEntity.h"

class ActionGrabEntity : public ActionUseEntity
{
	EntityHuman* m_human;
	EntityDynamic* m_entity;
	physx::PxJoint* m_joint;
	bool m_grabbed;

public:
	ActionGrabEntity(EntityHuman* human, EntityDynamic* entity);
	~ActionGrabEntity();

	void run() override;

	void startUse() override;
	void stopUse() override;
};