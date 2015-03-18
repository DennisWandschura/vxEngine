#pragma once

class TargetState;
class DecisionTreeNode;

#include "Transition.h"

class DecisionTreeTransition : public TransitionBase
{
	TargetState* m_pTargetState{ nullptr };
	DecisionTreeNode* m_pRoot;

public:
	DecisionTreeTransition(DecisionTreeNode* pRoot, TargetState* state = nullptr)
		:m_pTargetState(state), m_pRoot(pRoot){}

	Action* getAction() const override;

	State* getTargetState() const override;

	U8 isTriggered() override;
};