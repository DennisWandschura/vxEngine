#pragma once

class Condition;
class Action;
class State;

#include <vxLib/types.h>
#include <vector>

class TransitionBase
{
protected:
	~TransitionBase(){}

public:
	virtual U8 isTriggered() = 0;
	virtual State* getTargetState() const = 0;
	virtual Action* getAction() const = 0;
};

class Transition : public TransitionBase
{
	Action* m_actions;
	Condition* m_pCondition;
	State* m_pTargetState;

public:
	Transition(Condition* pCondition, State* pTargetState, Action* actions = nullptr) :
		m_actions(actions), m_pCondition(pCondition), m_pTargetState(pTargetState){}

	~Transition(){}

	U8 isTriggered() override;
	State* getTargetState() const override { return m_pTargetState; }
	Action* getAction() const override { return m_actions; }
};