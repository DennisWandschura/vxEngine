#include "StateMachine.h"
#include "Transition.h"
#include "State.h"

StateMachine::StateMachine()
	:m_states(),
	m_pInitialState(nullptr),
	m_pCurrentState(nullptr)
{
}

StateMachine::StateMachine(State* pInitialState, std::vector<State*> &&states)
	:m_states(std::move(states)),
	m_pInitialState(pInitialState),
	m_pCurrentState(pInitialState)
{

}

Action* StateMachine::update()
{
	TransitionBase* pTriggeredTransition = nullptr;

	auto &transitions = m_pCurrentState->getTransitions();
	for (auto &it : transitions)
	{
		if (it->isTriggered())
		{
			pTriggeredTransition = it;
			break;
		}
	}

	if (pTriggeredTransition)
	{
		auto targetState = pTriggeredTransition->getTargetState();

		//std::vector<Action*> actions;
		//actions.push_back(m_pCurrentState->getAction());

		m_pCurrentState = targetState;

		return m_pCurrentState->getAction();
	}
	else
	{
		return m_pCurrentState->getAction();
	}
}

void StateMachine::setInitialState(State* pState)
{
	m_pInitialState = pState;
	m_pCurrentState = pState;
}

void StateMachine::addState(State* pState)
{
	m_states.push_back(pState);
}