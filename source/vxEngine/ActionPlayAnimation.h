#pragma once

namespace physx
{
	class PxRigidDynamic;
}

#include "Action.h"
#include <vxEngineLib/Reference.h>
#include <vxEngineLib/Animation.h>

class ActionPlayAnimation : public Action
{
	Reference<vx::Animation> m_animation;
	u32 m_currentFrame;
	u32 m_frameCount;
	physx::PxRigidDynamic* m_rigidDynamic;

public:
	ActionPlayAnimation();
	~ActionPlayAnimation();

	void run() override;

	bool isComplete() const override;
};