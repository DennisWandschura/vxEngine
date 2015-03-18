#include "DecisionTreeTransition.h"
#include "TargetState.h"

Action* DecisionTreeTransition::getAction() const
{
	Action* action = nullptr;
	if (m_pTargetState)
	{
		action = m_pTargetState->getAction();
	}

	return action;
}

State* DecisionTreeTransition::getTargetState() const
{
	State* state = (m_pTargetState != nullptr) ? m_pTargetState->getTargetState() : nullptr;
	return state;
}

U8 DecisionTreeTransition::isTriggered()
{
	m_pTargetState = m_pRoot->makeDecision();

	return (m_pTargetState != nullptr);
}