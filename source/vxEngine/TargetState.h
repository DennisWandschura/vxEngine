#pragma once

class Action;
class State;

#include "DecisionTreeNode.h"

class TargetState : public DecisionTreeNode
{
	Action* m_pAction{ nullptr };
	State* m_pTargetState{ nullptr };

public:
	TargetState(Action* pAction, State* pState)
		:m_pAction(pAction), m_pTargetState(pState)
	{
	}

	TargetState* makeDecision() override
	{
		return this;
	}

	State* getTargetState() const { return m_pTargetState; }

	Action* getAction() { return m_pAction; }
};