/*
The MIT License (MIT)

Copyright (c) 2015 Dennis Wandschura

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
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